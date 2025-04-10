#include "vector.hpp"
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

static constexpr unsigned PORT = 80;
static constexpr size_t THREAD_COUNT = 20;

static volatile bool kill = false;

void *handle_client(void *args);

int main() {
  const auto accept_fd = socket(AF_INET, SOCK_STREAM, 0);

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

  // init worker threads
  fast::vector<pthread_t> thread_pool(THREAD_COUNT);
  // TODO

  printf("Server listening on %d\n", PORT);

  while (true) {
    const auto client = accept(accept_fd, NULL, NULL);

    if (client < 0) {
      perror("accept");
      close(accept_fd);
      break;
    } else {
      // send to thread
    }
  }

  for (const auto &t : thread_pool) {
    pthread_join();
  }
}
