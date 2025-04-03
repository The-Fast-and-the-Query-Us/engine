#include "constants.hpp"
#include "ranker.hpp"
#include <string_view.hpp>
#include <array.hpp>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
  
  const fast::string_view query = "trump";
  
  fast::array<fast::query::Result, fast::query::MAX_RESULTS> results;

  const auto fd = open("/tmp/index/0", O_RDONLY);

  if (fd == -1) {
    perror("Index open failed");
    exit(1);
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    perror("fail to get size");
    close(fd);
    exit(1);
  }

  const size_t chunk_size = sb.st_size;

  const auto map_ptr =
      mmap(nullptr, chunk_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (map_ptr == MAP_FAILED) [[unlikely]] {
    close(fd);
    perror("Fail to mmap index chunk");
    exit(1);
  }

  close(fd);

  auto blob = reinterpret_cast<const fast::hashblob *>(map_ptr);

  fast::query::blob_rank(blob, query, results);

  if (munmap(map_ptr, chunk_size) == -1) [[unlikely]] {
    perror("Fail to unmap chunk");
    exit(1);  // maybe shouldnt exit here?
  }

  for (const auto &r : results) {
    std::cout << "Url : " << r.second.c_str() << ", Rank : " << r.first << std::endl;
  }
}
