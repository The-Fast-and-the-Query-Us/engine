#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "query.hpp"

const int PORT = 8080;

int main(int argc, char **argv) {
  if (argc != 2) {
    perror("Useage: query <NUM_CHUNKS>");
    exit(1);
  }

  char *end;
  const auto NUM_CHUNKS = strtol(argv[1], &end, 10);

  if (end == argv[1] || *end != 0) {
    perror("Invlid number of chunks (should be a positive number)");
    exit(1);
  }

  const auto FD = socket(AF_INET, SOCK_STREAM, 0);

  if (FD < 0) {
    perror("socket(...)");
    exit(1);
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  if (bind(FD, (sockaddr*) &addr, sizeof(addr)) < 0) {
    close(FD);
    perror("bind(...)");
    exit(1);
  }

  // TODO how big should queue be
  if (listen(FD, 10) < 0) {
    close(FD);
    perror("listen(...)");
    exit(1);
  }

  printf("query listening on %d\n", PORT);

  // allow child to clean up immediately
  signal(SIGCHLD, SIG_IGN);

  while (true) {
    const auto client = accept(FD, NULL, NULL);
    if (client < 0) {
      close(FD);
      perror("accept(...)");
      exit(1);
    }

    if (fork() == 0) {

      close(FD); // both parent and child must close
      fast::query::handle(client, NUM_CHUNKS);
      close(client);
      return 0;

    } else {
      close(client);
    }
  }
}
