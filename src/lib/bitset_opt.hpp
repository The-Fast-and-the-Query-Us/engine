#include <cstddef>
#include <climits>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.hpp>

namespace fast {

class bitset_opt {
  using chunk_type = unsigned long long;
  static constexpr size_t BITS_PER_CHUNK = sizeof(unsigned long long) * CHAR_BIT;

  size_t num_bits{};
  size_t num_chunks{};
  chunk_type *data{};
  fast::string save_path;

  size_t block_index(size_t pos) const {
    return pos / BITS_PER_CHUNK;
  }

  size_t bit_offset(size_t pos) const {
    return pos % BITS_PER_CHUNK;
  }

public:
  bitset_opt() = default;

  bitset_opt(size_t _num_bits, const char* _save_path = nullptr)
    : num_bits(_num_bits),
    num_chunks((num_bits + BITS_PER_CHUNK - 1) / BITS_PER_CHUNK),
    data(new chunk_type[num_chunks]{}) {
    if (!_save_path)
      save_path = "";
    else
      save_path = _save_path;
  }

  bitset_opt(const char* load_path)
      : save_path(load_path == nullptr ? "" : load_path) {}

  bitset_opt(const bitset_opt& other)
    : num_bits(other.num_bits),
    num_chunks(other.num_chunks),
    data(other.data ? new chunk_type[other.num_chunks]{} : nullptr),
    save_path(other.save_path) {

    if (data)
      std::memcpy(data, other.data, num_chunks * sizeof(chunk_type));
  }

  bitset_opt &operator=(const bitset_opt& rhs) {
    if (this != &rhs) {
      delete[] data;
      num_bits = rhs.num_bits;
      num_chunks = rhs.num_chunks;
      data = rhs.data ? new chunk_type[num_chunks]{} : nullptr;
      save_path = rhs.save_path;
      if (data)
        std::memcpy(data, rhs.data, num_chunks * sizeof(chunk_type));
    }
    return *this;
  }

  ~bitset_opt() { delete[] data; }

  void set(size_t pos, bool value = true) {
    if (pos >= num_bits) return;
    if (value) {
      data[block_index(pos)] |= (static_cast<chunk_type>(1) << bit_offset(pos));
    } else {
      data[block_index(pos)] &= ~(static_cast<chunk_type>(1) << bit_offset(pos));
    }
  }

  bool test(size_t pos) const {
    if (pos >= num_bits) return false;
    return (data[block_index(pos)]
    & (static_cast<chunk_type>(1) << bit_offset(pos))) != 0;
  }

  size_t size() const {
    return num_bits;
  }

  int save(int pos = 0) {
    int fd = open(save_path.begin(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
      perror(save_path.begin());
      throw std::runtime_error("Failed to open save_path");
    }
    int num_seek = lseek(fd, pos, SEEK_SET);
    if (num_seek != pos)
      throw std::runtime_error("Failed to seek to pos in save");

    if (fd == -1) {
      std::cerr << "Failed to open bitset dump file on write: " << save_path.begin()
                << "\n";
      return -1;
    }

    ssize_t blocks_written = write(fd, &num_chunks, sizeof(uint64_t));
    if (blocks_written == -1) {
      perror("Write in save for bitset failed on member num_chunks");
      close(fd);
      return -1;
    }

    ssize_t num_bits_written = write(fd, &num_bits, sizeof(uint64_t));
    if (num_bits_written == -1) {
      perror("Write in save for bitset failed on member num_bits");
      close(fd);
      return -1;
    }

    ssize_t elts_written = write(fd, data, num_chunks * sizeof(uint64_t));
    if (elts_written == -1) {
      perror("Write in save for bitset failed on member bits");
      close(fd);
      return -1;
    }

    if (close(fd) == -1) {
      perror("Error closing file in bitset save()");
    }

    std::cout << "Successfully wrote bitset to " << save_path.begin() << '\n';

    return blocks_written + num_bits_written + elts_written;
  }

  int load(int pos = 0) {
    if (save_path == nullptr) {
      perror("save_path=nullptr\n");
      return -1;
    }
    int fd = open(save_path.begin(), O_RDONLY);
    if (fd < 0) {
      std::cerr << "error opening save_path: " << save_path.begin() << '\n';
      return -1;
    }
    int offset = lseek(fd, pos, SEEK_SET);
    if (offset == off_t(-1)) {
      std::cerr << "error seeking: " << strerror(errno) << '\n';
      std::cerr << "save_path: " << save_path.begin() << '\n';
      close(fd);
      return -1;
    }

    if (fd == -1)
      throw std::runtime_error("Failed to open bitset dump file on read");

    ssize_t blocks_read = read(fd, &num_chunks, sizeof(uint64_t));
    if (blocks_read == -1) {
      close(fd);
      throw std::runtime_error("Read in load for bitset failed (length)");
    }

    ssize_t num_bits_read = read(fd, &num_bits, sizeof(uint64_t));
    if (num_bits_read == -1) {
      close(fd);
      throw std::runtime_error("Read in load for bitset failed (length)");
    }

    delete[] data;
    data = new uint64_t[num_chunks]();

    ssize_t elts_read = read(fd, data, num_chunks * sizeof(uint64_t));
    if (elts_read == -1) {
      close(fd);
      throw std::runtime_error("Read in load for bitset failed\n");
    }

    if (close(fd) == -1) {
      throw std::runtime_error("Error closing file in bitset load()\n");
    }

    return blocks_read + num_bits_read + elts_read;
  }
};

}
