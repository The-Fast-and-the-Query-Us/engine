#include "array.hpp"
#include "condition_variable.hpp"
#include "constants.hpp"
#include "hashblob.hpp"
#include "language.hpp"
#include "mutex.hpp"
#include "network.hpp"
#include "queue.hpp"
#include "ranker.hpp"
#include "string.hpp"
#include "string_view.hpp"
#include "vector.hpp"
#include <cstdio>
#include <cstring>
#include <functional>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

constexpr unsigned PORT = 8081;
constexpr size_t ACCPETOR_THREADS = 20; // this must be at least the same as the frontend

constexpr bool LOCK_MEM = false; // false => fast start up and slow query

volatile bool die = false;
fast::queue<int> clients;
fast::mutex mtx;
fast::condition_variable cv;

fast::vector<const fast::hashblob*> blobs;

/*
* Read in all blobs and add to the vector of blobs
*/
void init_blobs() {
  fast::string chunk_count_path = getenv("HOME");
  chunk_count_path += "/.local/share/crawler/chunk_count.bin";

  const auto fd = open(chunk_count_path.c_str(), O_RDONLY);
  if (fd < 0) {
    perror("fail to find chunk count");
    exit(1);
  }

  uint64_t chunk_count;
  assert(read(fd, &chunk_count, sizeof(chunk_count)) == sizeof(chunk_count) &&
         "invalid chunk_count");
  close(fd);

  fast::string base = getenv("HOME");
  base += "/.local/share/crawler/index/";

  for (uint64_t chunk_num = 0; chunk_num < chunk_count; ++chunk_num) {
    const auto chunk_path = base + fast::to_string(chunk_num);
    const auto chunk_fd = open(chunk_path.c_str(), O_RDONLY);

    if (chunk_fd < 0) {
      perror("unable to open chunk");
      exit(1);
    }

    struct stat sb;
    if (fstat(chunk_fd, &sb) < 0) {
      perror("fail to get size");
      close(chunk_fd);
      exit(1);
    }

    const size_t chunk_size = sb.st_size;
    const auto map_ptr =
        mmap(nullptr, chunk_size, PROT_READ, MAP_PRIVATE, chunk_fd, 0);

    close(chunk_fd);

    if (map_ptr == MAP_FAILED) [[unlikely]] {
      perror("Fail to mmap index chunk");
      exit(1);
    }

    // this blocks until page faults are done
    if constexpr (LOCK_MEM) {
      if (mlock(map_ptr, chunk_size) != 0) {
        perror("Fail to lock chunk");
        exit(1); // exit fotr debuging (remove later)
      }
    }

    auto blob = reinterpret_cast<const fast::hashblob*>(map_ptr);

    if (!blob->is_good()) {
      fprintf(stderr, "BAD CHUNK");
    } else {
      blobs.push_back(blob);
    }
  }
}

static constexpr size_t RANKER_THREADS = 20;

struct ranker_args {
  size_t start;
  const fast::string *query;
  const fast::vector<fast::string_view> *flattened;
};

void *ranker_thread(void *args) {
  ranker_args *ra = reinterpret_cast<ranker_args *>(args);

  auto ans = new fast::array<fast::query::Result, fast::query::MAX_RESULTS>;
  for (size_t i = ra->start; i < blobs.size(); i += RANKER_THREADS) {
    fast::query::query_stream qs(*ra->query);
    auto constraints =
        fast::query::contraint_parser::parse_contraint(qs, blobs[i]);

    if (constraints) {
      fast::query::rank(blobs[i], *ra->flattened, constraints, *ans);
    }
    delete constraints;
  }

  return ans;
}

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

      fast::vector<fast::pair<ranker_args, pthread_t>> rt;

      fast::query::query_stream qs(query);
      fast::vector<fast::string_view> flattened;
      fast::query::rank_parser::parse_query(qs, &flattened);

      rt.resize(RANKER_THREADS);

      for (size_t i = 0; i < RANKER_THREADS; ++i) {
        rt[i].first.flattened = &flattened;
        rt[i].first.query = &query;
        rt[i].first.start = i;

        assert(
          !pthread_create(&rt[i].second, NULL, ranker_thread, &rt[i].first)
        );
      }

      fast::array<fast::query::Result, fast::query::MAX_RESULTS> results;

      for (size_t i = 0; i < RANKER_THREADS; ++i) {
        fast::array<fast::query::Result, fast::query::MAX_RESULTS> *ptr;
        pthread_join(rt[i].second, (void**) &ptr);

        for (const auto &r : *ptr) {
          fast::query::insertion_sort(results, r);
        }

        delete ptr;
      }

      uint32_t count = 0;
      for (const auto &r : results) {
        if (r.second.size() > 0) ++count;
      }

      fast::send_all(client, count);

      for (const auto &r : results) {
        if (r.second.size() > 0) {
          float rank = r.first;
          uint32_t buf;
          memcpy(&buf, &rank, sizeof(buf));

          fast::send_all(client, buf);
          fast::send_all(client, r.second);
        }
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
  fast::vector<pthread_t> workers(ACCPETOR_THREADS);

  for (auto &t : workers) {
    pthread_create(&t, NULL, worker, NULL);
  }

  init_blobs(); // maybe unmap

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
