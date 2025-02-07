#pragma once

#include "bitset.hpp"
#include "murmur_hash3.hpp"
#include "pair.hpp"
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <mutex.hpp>
#include <openssl/md5.h>
#include <stdexcept>
#include <type_traits>

namespace fast {

template <typename T>
concept primitive = std::is_arithmetic_v<T>;

template <typename T>
concept complex = requires(T v) {
  { v.data() } -> std::convertible_to<const void *>;
  { v.size() } -> std::convertible_to<size_t>;
};

template <typename T>
concept hashable = primitive<T> || complex<T>;

template <typename T>
class bloom_filter {
public:
  bloom_filter(size_t _n, double _fpr) : n(_n), fpr(_fpr) {
    if (n == 0) {
      throw std::invalid_argument("n must be greater than 0");
    }
    if (fpr <= 0 || fpr >= 1) {
      throw std::invalid_argument("fpr must be in the interval (0, 1)");
    }

    const double ln2 = std::log(2);
    const double ln_fpr = std::log(fpr);
    num_bits = (-1 * static_cast<int>(n) * ln_fpr) / (ln2 * ln2);
    bit_set = bitset(num_bits);

    num_hash = (num_bits / n) * ln2;
  }

  void insert(const T &val) {
    auto [h1, h2] = hash(val);
    m.lock();
    for (size_t i = 0; i < num_hash; ++i) {
      bit_set[double_hash(h1, h2, i)] = 1;
    }
    m.unlock();
  }

  bool contains(const T &val) {
    auto [h1, h2] = hash(val);
    m.lock();
    for (size_t i = 0; i < num_hash; ++i) {
      if (!bit_set[double_hash(h1, h2, i)])
        return false;
    }
    m.unlock();
    return true;
  }

private:
  size_t n;

  uint64_t num_bits;

  double fpr;

  uint64_t num_hash;

  bitset bit_set;

  fast::mutex m;

  pair<const unsigned char *, size_t> serialize(const T &datum) {
    if constexpr (complex<T>)
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
