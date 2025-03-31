#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>

static constexpr int MAX_PATH = 4096;
static const int PORT = 8080;

static int FD;
static int NUM_CHUNKS;
static char INDEX_DIR[MAX_PATH + 1];
static char *DIR_END;

// close FD on SIGINT
static void handle_cleanup(int signal) {
  if (signal == SIGINT) {
    close(FD);
    exit(0);
  }
}

// TODO
static void handle_client(const int fd) {

}

int main(int argc, char **argv) {
  if (argc != 3) {
    perror("Useage: query <NUM_CHUNKS> <index_path>");
    exit(1);
  }

  char *end;
  NUM_CHUNKS = strtol(argv[1], &end, 10);

  if (end == argv[1] || *end != 0) {
    perror("Invlid number of chunks (should be a positive number)");
    exit(1);
  }

  // check that directory exists
  const auto dir = opendir(argv[2]);
  if (!dir) {
    perror("Invalid index path");
    exit(1);
  }
  closedir(dir);

  DIR_END = INDEX_DIR;
  for (auto ptr = argv[2]; *ptr; ++ptr, ++DIR_END) *DIR_END = *ptr;
  *(DIR_END++) = '/';

  FD = socket(AF_INET, SOCK_STREAM, 0);
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
  signal(SIGINT, handle_cleanup);

  while (true) {
    const auto client = accept(FD, NULL, NULL);
    if (client < 0) {
      close(FD);
      perror("accept(...)");
      exit(1);
    }

    // use fork to prevent sharing address space
    if (fork() == 0) {

      close(FD); // both parent and child must close
      handle_client(client);
      close(client);
      return 0;

    } else {
      close(client);
    }
  }
}
