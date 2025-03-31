#include <dirent.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array.hpp>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <hashblob.hpp>
#include <network.hpp>
#include <string.hpp>

#include "constants.hpp"

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

// todo
void blob_rank(const fast::hashblob *, const fast::string &,
               fast::array<fast::query::Result, fast::query::MAX_RESULTS>) {
  return;
}

static void handle_client(const int client_fd) {
  fast::string query;
  fast::recv_all(client_fd, query);  // Check return?

  fast::array<fast::query::Result, fast::query::MAX_RESULTS> results;

  for (auto chunk_num = 0; chunk_num < NUM_CHUNKS; ++chunk_num) {
    sprintf(DIR_END, "%d", chunk_num);
    const int fd = open(INDEX_DIR, O_RDONLY);

    if (fd == -1) [[unlikely]] {
      perror("Fail to open index chunk");
      fprintf(stderr, "Failed to open index chunk : %d\n", chunk_num);
      exit(1);  // should we continue?
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) [[unlikely]] {
      close(fd);
      perror("Fail to get chunk stats");
      fprintf(stderr, "Failed to get stat for chunk : %d\n", chunk_num);
      exit(1);
    }
    const size_t chunk_size = sb.st_size;

    const auto map_ptr =
        mmap(nullptr, chunk_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_ptr == MAP_FAILED) [[unlikely]] {
      close(fd);
      perror("Fail to mmap index chunk");
      fprintf(stderr, "Fail to mmap chunk number : %d\n", chunk_num);
      exit(1);
    }

    close(fd);

    auto blob = reinterpret_cast<const fast::hashblob *>(map_ptr);

    blob_rank(blob, query, results);

    if (munmap(map_ptr, chunk_size) == -1) [[unlikely]] {
      perror("Fail to unmap chunk");
      exit(1);  // maybe shouldnt exit here?
    }
  }

  // send results
  size_t count = 0;
  for (const auto &result : results) {
    if (result.second.size() > 0) ++count;
  }

  fast::send_all(client_fd, count);
  for (const auto &result : results) {
    if (result.second.size() > 0) {
      fast::send_all(client_fd, result.first);
      fast::send_all(client_fd, result.second);
    }
  }
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

  if (bind(FD, (sockaddr *)&addr, sizeof(addr)) < 0) {
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
      close(FD);  // both parent and child must close
      handle_client(client);
      close(client);
      return 0;

    } else {
      close(client);
    }
  }
}
