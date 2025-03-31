#include <cstdio>
#include <isr.hpp>
#include <string.hpp>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace fast::query {

static string get_query(const int client_fd);

void handle(const int client_fd, const int num_chunks, const char *index_dir, char *dir_end) {
  const auto query = get_query(client_fd);

  for (auto chunk_num = 0; chunk_num < num_chunks; ++chunk_num) {
    sprintf(dir_end, "%d", chunk_num);
    const int fd = open(index_dir, O_RDONLY);

    if (fd == -1) {
      perror("Fail to open index chunk");
      fprintf(stderr, "Failed to open index chunk : %d\n", chunk_num);
      exit(1); // should we continue?
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      perror("Fail to get chunk stats");
      fprintf(stderr, "Failed to get stat for chunk : %d\n", chunk_num);
      exit(1);
    }
    const size_t chunk_size = sb.st_size;

    close(fd);
  }
}

}
