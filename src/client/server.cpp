#include "array.hpp"
#include "condition_variable.hpp"
#include "mutex.hpp"
#include "network.hpp"
#include "pair.hpp"
#include "queue.hpp"
#include "string.hpp"
#include "vector.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include "crawler.hpp"

constexpr unsigned PORT = 80;
constexpr unsigned QUERY_PORT = 8081;

constexpr size_t THREAD_COUNT = 20;

volatile bool quit = false;
fast::mutex mtx;
fast::condition_variable cv;
fast::queue<int> clients;

fast::string html;
fast::string img;

fast::vector<fast::string> ips;

const bool logging = false;

/*
* Read in files to serve as frontend
*/
static void init_files() {
  char buffer[1 << 10];

  auto fd = open("frontend/html/timer.html", O_RDONLY);
  if (fd < 0) {
    perror("Fail to open index.html");
    exit(1);
  }

  for (auto r = read(fd, buffer, sizeof(buffer)); r > 0; r = read(fd, buffer, sizeof(buffer))) {
    html += fast::string(buffer, r);
  }

  close(fd);

  fd = open("frontend/img/search_engine.png", O_RDONLY);
  if (fd < 0) {
    perror("Fail to open img");
    exit(1);
  }

  for (auto r = read(fd, buffer, sizeof(buffer)); r > 0; r = read(fd, buffer, sizeof(buffer))) {
    img += fast::string(buffer, r);
  }

  close(fd);
}

static void serve_file(const int fd, const fast::string &file, const fast::string &type) {

  fast::string response = "HTTP/1.1 200 OK\r\n";
  response = response + "Content-Type: " + type + "\r\n";
  response = response + "Content-Length: " + fast::to_string(file.size()) + "\r\n";
  response = response + "Access-Control-Allow-Origin: *\r\n";
  response += "\r\n";

  fast::send_all(fd, response.c_str(), response.size());
  fast::send_all(fd, file.c_str(), file.size());
}

static void send_results(const fast::array<fast::pair<fast::string, float>, 10> &results, const int fd) {
  fast::string arr = "[";

  for (const auto &p : results) {
    if (p.first.size() == 0) continue;
    arr = arr + ((arr.size() == 1) ? "" : ", ") + "\"" + p.first + "\"";
  }
  arr += "]";

  fast::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n";

  response = response + "Content-Length: " + fast::to_string(arr.size()) + "\r\n\r\n";

  fast::send_all(fd, response.c_str(), response.size());
  fast::send_all(fd, arr.c_str(), arr.size());
}

static void serve_query(const int fd, const fast::string_view &query, const fast::vector<int> servers) {
  fast::string translated;
  translated.reserve(query.size());

  //handle URL encoding
  for (const auto c : query) {
    translated += c;
    if (translated.ends_with("%20")) {
      translated.pop_back(3);
      translated += ' ';
    }
    else if (translated.ends_with("%5B")) {
      translated.pop_back(3);
      translated += '[';
    }
    else if (translated.ends_with("%5D")) {
      translated.pop_back(3);
      translated += ']';
    }
    else if (translated.ends_with("%22")) {
      translated.pop_back(3);
      translated += '"';
    }
  }

  for (auto &c : translated) {
    c = tolower(c);
  }

  // short circuit stop word queries
  if (translated == "the" || translated == "and" || translated == "is") {

    if constexpr (logging)
      std::cout << "Short circuit response for " << translated.c_str() << std::endl;

    static fast::array<fast::pair<fast::string, float>, 10>  base;
    base[0].first = "https://en.wikipedia.org/wiki/";
    base[1].first = "https://www.dictionary.com/browse/";
    base[2].first = "https://www.merriam-webster.com/dictionary/";
    base[3].first = "https://en.wiktionary.org/wiki/";
    base[4].first = "https://dictionary.cambridge.org/us/dictionary/english/";
    base[5].first = "https://www.collinsdictionary.com/us/dictionary/english/";

    auto added = base;

    for (auto &r : added) {
      r.first += translated;
    }

    send_results(added, fd);
    return;
  }

  if (logging) {
    std::cout << "Forwarding request: " << translated.c_str() << std::endl;
  }

  for (const auto s : servers) {
    if (!fast::send_all(s, translated)) {
      if (logging) {
        std::cout << "Fail to send to query server" << std::endl;
      }
    }
  }

  fast::array<fast::pair<fast::string, float>, 10> results;

  for (const auto s : servers) {
    uint32_t count;
    fast::recv_all(s, count);

    while (count--) {
      uint32_t rank;
      fast::string url;

      fast::recv_all(s, rank);
      fast::recv_all(s, url);


      float casted;
      memcpy(&casted, &rank, sizeof(casted));

      if (logging)
        std::cout << url.c_str() << " with rank: " << casted << std::endl;

      if (casted > results[9].second) {
        results[9] = {url, casted};

        for (size_t i = 9; i > 0; --i) {
          if (results[i].second > results[i - 1].second) {
            fast::swap(results[i], results[i - 1]);
          } else {
            break;
          }
        }
      }
    }
  }

  send_results(results, fd);
}

static void serve_not_found(const int fd) {
  fast::string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
  fast::send_all(fd, response.c_str(), response.size());
}

static void serve_client(const int fd, const fast::vector<int> &servers) {
  char buffer[1 << 10]{}; // 4KB
  const auto header = recv(fd, buffer, sizeof(buffer), 0);

  size_t start = 0;
  while (start < header && buffer[start++] != '/');

  size_t end = start;
  while (end < header && buffer[end] != ' ') ++end;

  const fast::string path(buffer + start, buffer + end);

  if (path.starts_with("api?q=")) {
    serve_query(fd, path.view().trim_prefix(6), servers);
  } else if (path.size() == 0) {
    serve_file(fd, html, "text/html");
  } else if (path == "img") {
    serve_file(fd, img, "image/png");
  } else {
    serve_not_found(fd);
  }
}

void *worker(void*) {
  fast::vector<int> servers;

  for (const auto &ip : ips) {
    const auto sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
      perror("worker::socket");
      exit(1);
    }

    struct sockaddr_in addr{};
    addr.sin_port = htons(QUERY_PORT);
    addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) < 0) {
      perror("inet pton");
      close(sock);
      exit(1);
    }

    if (connect(sock, (sockaddr*) &addr, sizeof(addr)) < 0) {
      perror("Server unreachable");
      close(sock);
      continue;
    }

    servers.push_back(sock);
  }

  while (true) {
    mtx.lock();

    while (!quit && clients.empty())  cv.wait(&mtx);

    if (quit)  {
      mtx.unlock();

      for (const auto s : servers) {
        close(s);
      }

      return NULL;
    }

    const auto client = clients.front();
    clients.pop();
    mtx.unlock();

    serve_client(client, servers);
    close(client);
  }
}

int accept_fd;

int main() {
  init_files();
  const auto ip_file = fopen("ips.txt", "r");

  if (ip_file == NULL) {
    perror("Cannot find ip file");
    exit(1);
  }

  char buffer[40]{}; // should be long enough for any ip addr
  while (fgets(buffer, sizeof(buffer), ip_file) != NULL) {
    buffer[strcspn(buffer, "\r\n")] = 0;

    if (buffer[0]) 
      ips.emplace_back(buffer);
  }

  fclose(ip_file);

  accept_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (accept_fd < 0) {
    perror("socket()");
    exit(1);
  }

  int yes = 1;
  if (setsockopt(accept_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    perror("setsockopt()");
    close(accept_fd);
    exit(1);
  }

  struct sockaddr_in addr{};

  addr.sin_port = htons(PORT);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(accept_fd, (const sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("bind()");
    close(accept_fd);
    exit(1);
  }

  if (listen(accept_fd, SOMAXCONN) < 0) {
    perror("listen");
    close(accept_fd);
    exit(1);
  }

  fast::vector<pthread_t> thread_pool(THREAD_COUNT);
  for (auto &t : thread_pool) {
    if (pthread_create(&t, NULL, worker, NULL) != 0) {
      perror("Fail to create thread"); // should we exit here?
    }
  }

  signal(SIGINT, [](const int){
    shutdown(accept_fd, 0);
    close(accept_fd);
  });

  printf("Server listening on %d\n", PORT);

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

  quit = true;
  cv.broadcast();

  for (const auto &t : thread_pool) {
    pthread_join(t, NULL);
  }
}
