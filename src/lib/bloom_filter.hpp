#pragma once

#include "bitset.hpp"
#include "murmur_hash3.hpp"
#include "pair.hpp"
#include "scoped_lock.hpp"
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/fcntl.h>
#include <type_traits>
#include <unistd.h>

namespace fast {

template <typename T>
concept primitive = std::is_arithmetic_v<T>;

template <typename T>
concept container = requires(T v) {
  { v.data() } -> std::convertible_to<const void *>;
  { v.size() } -> std::convertible_to<size_t>;
};

template <typename T>
concept hashable = primitive<T> || container<T>;

template <typename T>
class bloom_filter {
public:
  bloom_filter(size_t _n, double _fpr, const char *_save_path)
      : n(_n), fpr(_fpr), save_path(_save_path) {
    init();

    bit_set = bitset(num_bits, save_path);
  }

  bloom_filter(const char *load_path) { load(load_path); }

  void insert(const T &val) {
    auto [h1, h2] = hash(val);
    fast::scoped_lock lock_guard(&m);
    for (size_t i = 0; i < num_hash; ++i) {
      bit_set[double_hash(h1, h2, i)] = 1;
    }
  }

  bool contains(const T &val) {
    auto [h1, h2] = hash(val);
    fast::scoped_lock lock_guard(&m);
    for (size_t i = 0; i < num_hash; ++i) {
      if (!bit_set[double_hash(h1, h2, i)])
        return false;
    }
    return true;
  }

  int save() {
    fast::scoped_lock lock_guard(&m);
    int bs_sz = bit_set.save();

    int fd = open(save_path, O_WRONLY);
    if (fd == -1) {
      std::cerr << "Failed to open bitset dump file on write\n";
      return -1;
    }

    int offset = lseek(fd, bs_sz, SEEK_SET);
    if (offset == off_t(-1)) {
      std::cerr << "error seeking: " << strerror(errno) << std::endl;
      close(fd);
      return -1;
    }

    ssize_t n_written = write(fd, &n, sizeof(size_t));
    if (n_written == (-1)) {
      std::cerr << "error writing member n: " << strerror(errno) << std::endl;
      close(fd);
      return -1;
    }

    ssize_t fpr_written = write(fd, &fpr, sizeof(double));
    if (fpr_written == (-1)) {
      std::cerr << "error writing member fpr: " << strerror(errno) << std::endl;
      close(fd);
      return -1;
    }

    if (close(fd) == -1) {
      std::cerr << "error closing file in bloom_filter save()";
      return -1;
    }

    return bs_sz + n_written + fpr_written;
  }

  int load(const char *load_path) {
    fast::scoped_lock lock(&m);

    int fd = open(load_path, O_RDONLY);
    if (fd == -1)
      throw std::runtime_error("Failed to open bloom_filter dump file on read");

    save_path = load_path;

    int bs_sz = bit_set.load(fd);

    fd = open(save_path, O_RDONLY);
    int offset = lseek(fd, bs_sz, SEEK_SET);
    if (offset == off_t(-1)) {
      close(fd);
      throw std::runtime_error("error seeking in bloom_filter load()");
    }

    ssize_t n_read = read(fd, &n, sizeof(size_t));

    if (n_read == (-1)) {
      close(fd);
      throw std::runtime_error("error reading member n in bloom_filter load()");
    }

    ssize_t fpr_read = read(fd, &fpr, sizeof(double));
    if (fpr_read == (-1)) {
      close(fd);
      throw std::runtime_error(
          "error reading member fpr in bloom_filter load()");
    }

    if (close(fd) == -1)
      throw std::runtime_error("Error closing file in bloom_filter load()");

    init();

    return bs_sz + n_read + fpr_read;
  }

private:
  size_t n;

  uint64_t num_bits;

  double fpr;

  uint64_t num_hash;

  fast::bitset bit_set;

  fast::mutex m;

  const char *save_path;

  void init() {
    if (n == 0) {
      throw std::invalid_argument("n must be greater than 0");
    }
    if (fpr <= 0 || fpr >= 1) {
      throw std::invalid_argument("fpr must be in the interval (0, 1)");
    }

    const double ln2 = std::log(2);
    const double ln_fpr = std::log(fpr);
    num_bits = (-1 * static_cast<double>(n) * ln_fpr) / (ln2 * ln2);

    num_hash = static_cast<uint64_t>((static_cast<double>(num_bits) / n) * ln2);
  }

  pair<const unsigned char *, size_t> serialize(const T &datum) {
    if constexpr (container<T>)
      return pair{reinterpret_cast<const unsigned char *>(datum.data()),
                  datum.size()};
    else if constexpr (primitive<T>)
      return pair{reinterpret_cast<const unsigned char *>(&datum),
                  sizeof(datum)};
    else
      throw std::runtime_error("The type provided cannot be hashed.");
  }

  size_t double_hash(uint64_t h1, uint64_t h2, uint32_t i) {
    return (h1 + i * h2) % num_bits;
  }

  pair<uint64_t, uint64_t> hash(const T &datum) {
    auto [serialized, len] = serialize(datum);

    uint64_t curr_hash[2];
    MurmurHash3_x86_128(serialized, len, 0, curr_hash);

    return pair(curr_hash[0], curr_hash[1]);
  }
};

} // namespace fast
