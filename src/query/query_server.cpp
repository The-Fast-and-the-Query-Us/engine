#include "array.hpp"
#include "condition_variable.hpp"
#include "constants.hpp"
#include "mutex.hpp"
#include "network.hpp"
#include "queue.hpp"
#include "ranker.hpp"
#include "string.hpp"
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr unsigned PORT = 8081;
constexpr size_t THREAD_COUNT = 20;

volatile bool die = false;
fast::queue<int> clients;
fast::mutex mtx;
fast::condition_variable cv;

void *worker(void*) {
  while (true) {
    mtx.lock();
    while (!die && clients.empty()) cv.wait(&mtx);

    if (die) {
      mtx.unlock();
      return NULL;
    }

    const auto client = clients.front();
    clients.pop();
    mtx.unlock();

    while (true) {
      fast::string query;
      if (!fast::recv_all(client, query)) break;

      fast::array<fast::query::Result, fast::query::MAX_RESULTS> results;
      std::function<void(const fast::query::Result&)> call_back = [&results](const auto &r) {
        if (results[fast::query::MAX_RESULTS - 1].first < r.first) {
          results[fast::query::MAX_RESULTS - 1] = r;

          for (size_t i = fast::query::MAX_RESULTS - 1; i > 0; --i) {
            if (results[i].first > results[i - 1].first) {
              swap(results[i], results[i - 1]);
            } else {
              break;
            }
          }
        }
      };

      fast::query::rank_all(query, call_back);

      uint32_t count = 0;
      for (const auto &r : results) {
        if (r.second.size() > 0) ++count;
      }

      fast::send_all(client, count);

      for (const auto &r : results) {
        float rank = r.first;
        uint32_t buf;
        memcpy(&buf, &rank, sizeof(buf));

        std::cout << r.second.c_str() << std::endl;

        fast::send_all(client, buf);
        fast::send_all(client, r.second);
      }
    }

    close(client);
  }
}

int closer = -1;

int main() {
  const auto accept_fd = socket(AF_INET, SOCK_STREAM, 0);
  closer = accept_fd;

  if (accept_fd < 0) {
    perror("SOCKET FAIL");
    exit(1);
  }

  struct sockaddr_in addr{};

  addr.sin_port = htons(PORT);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(accept_fd, (sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("Bind");
    close(accept_fd);
    exit(1);
  }

  if (listen(accept_fd, SOMAXCONN) < 0) {
    perror("listen");
    close(accept_fd);
    exit(1);
  }

  signal(SIGINT, [](const int i) {
    shutdown(closer, O_RDWR);
    close(closer);
  });

  // init thread pool
  fast::vector<pthread_t> workers(THREAD_COUNT);

  for (auto &t : workers) {
    pthread_create(&t, NULL, worker, NULL);
  }

  while (true) {
    const auto client = accept(accept_fd, NULL, NULL);

    if (client < 0) {

      perror("accept");
      break;

    } else {

      mtx.lock();
      clients.push(client);
      mtx.unlock();
      cv.signal();

    }
  }

  die = true;
  cv.broadcast();
  for (const auto &t : workers) {
    pthread_join(t, NULL);
  }
}
