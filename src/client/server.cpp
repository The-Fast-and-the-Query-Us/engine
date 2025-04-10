#include "condition_variable.hpp"
#include "mutex.hpp"
#include "network.hpp"
#include "queue.hpp"
#include "string.hpp"
#include "vector.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

constexpr unsigned PORT = 80;
constexpr size_t THREAD_COUNT = 20;

volatile bool quit = false;

fast::mutex mtx;
fast::condition_variable cv;
fast::queue<int> clients;

const char *get_type(const fast::string &file) {
  if (file.ends_with(".html")) {
    return "text/html";
  } else if (file.ends_with(".png")) {
    return "image/png";
  } else {
    return ""; // TODO
  }
}

void serve_file(const int fd, const fast::string &path) {
  char buffer[1 << 10]{};
  const int file = open(path.c_str(), O_RDONLY);

  if (file < 0) {

    fast::string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
    fast::send_all(fd, response.c_str(), response.size());

  } else {

    struct stat sb;
    fstat(file, &sb);

    fast::string response = "HTTP/1.1 200 OK\r\n";
    response = response + "Content-Type: " + get_type(path) + "\r\n";
    response = response + "Content-Length: " + fast::to_string(sb.st_size) + "\r\n";
    response += "\r\n";

    fast::send_all(fd, response.c_str(), response.size());
    
    int red;
    while ((red = read(file, buffer, sizeof(buffer))) && red > 0) {
      fast::send_all(fd, buffer, red);
    }

    close(file);
  }
}

void serve_client(const int fd) {
  char buffer[1 << 12]{}; // 4KB
  const auto header = recv(fd, buffer, sizeof(buffer), 0);

  size_t start = 0;
  while (start < header && buffer[start++] != '/');

  size_t end = start;
  while (end < header && buffer[end] != ' ') ++end;

  const fast::string path(buffer + start, buffer + end);

  if (path == "api") { // todo
    // fan query
  } else if (path.size() == 0) {
    serve_file(fd, "frontend.html");
  } else if (path == "img") {
    serve_file(fd, "search_engine.png");
  }
}

void *worker(void*) {
  while (true) {
    mtx.lock();

    while (!quit && clients.empty())  cv.wait(&mtx);

    if (quit)  {
      mtx.unlock();
      return NULL;
    }

    const auto client = clients.front();
    clients.pop();
    mtx.unlock();

    serve_client(client);
    close(client);
  }
}

int accept_fd;

int main() {
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
