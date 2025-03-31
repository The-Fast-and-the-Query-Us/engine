#include <cstdio>
#include <isr.hpp>
#include <string.hpp>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <hashblob.hpp>

namespace fast::query {

static string get_query(const int client_fd);

void handle(const int client_fd, const int num_chunks, const char *index_dir, char *dir_end) {
  const auto query = get_query(client_fd);

  for (auto chunk_num = 0; chunk_num < num_chunks; ++chunk_num) {
    sprintf(dir_end, "%d", chunk_num);
    const int fd = open(index_dir, O_RDONLY);

    if (fd == -1) [[unlikely]] {
      perror("Fail to open index chunk");
      fprintf(stderr, "Failed to open index chunk : %d\n", chunk_num);
      exit(1); // should we continue?
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) [[unlikely]] {
      close(fd);
      perror("Fail to get chunk stats");
      fprintf(stderr, "Failed to get stat for chunk : %d\n", chunk_num);
      exit(1);
    }
    const size_t chunk_size = sb.st_size;

    const auto map_ptr = mmap(nullptr, chunk_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_ptr == MAP_FAILED) [[unlikely]] {
      close(fd);
      perror("Fail to mmap index chunk");
      fprintf(stderr, "Fail to mmap chunk number : %d\n", chunk_num);
      exit(1);
    }

    close(fd);

    auto blob = reinterpret_cast<const hashblob*>(map_ptr);
    

    // do processing

    if (munmap(map_ptr, chunk_size) == -1) [[unlikely]] {
      perror("Fail to unmap chunk");
      exit(1);
    }
  }
}

}
