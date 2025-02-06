#pragma once

#include "bitset.hpp"
#include "pair.hpp"
#include <cmath>
#include <concepts>
#include <cstring>
#include <openssl/md5.h>
#include <stdexcept>
#include <type_traits>
#include <mutex.hpp>

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

template <hashable T>
class bloom_filter {
public:
  bloom_filter(size_t _n, double _fpr) : n(_n), fpr(_fpr) {
    const double log2 = std::log(2);

    num_bits = (-1 * n * std::log(fpr)) / (std::pow(log2, 2));
    bit_set = bitset(num_bits);

    num_hash = (num_bits / n) * log2;
  }

  void insert(const T &val) {
    auto [h1, h2] = hash(val);
    m.lock();
    for (int i = 0; i < num_hash; ++i) {
      bit_set[double_hash(h1, h2, i)] = 1;
    }
    m.unlock();
  }

  bool contains(const T &val) {
    auto [h1, h2] = hash(val);
    m.lock();
    for (int i = 0; i < num_hash; ++i) {
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

  pair<unsigned char *, size_t> serialize(const T &datum) {
    if constexpr (complex<T>)
      return {reinterpret_cast<unsigned char *>(datum.data()), datum.size()};
    else if constexpr (primitive<T>)
      return {reinterpret_cast<unsigned char *>(&datum), sizeof(datum)};
    else
      throw std::runtime_error("The type provided cannot be hashed.");
  }

  size_t double_hash(uint64_t h1, uint64_t h2, uint32_t i) {
    return (h1 + i * h2) % num_bits;
  }

  pair<uint64_t, uint64_t> hash(const T &datum) {
    auto [serialized, len] = serialize(datum);

    unsigned char curr_hash[MD5_DIGEST_LENGTH];
    MD5(serialized, len, curr_hash);

    uint64_t h1, h2;
    std::memcpy(&h1, curr_hash, sizeof(uint64_t)); // first 8 bytes
    std::memcpy(&h2, curr_hash + sizeof(uint64_t),
                sizeof(uint64_t)); // next 8 bytes

    return pair(h1, h2);
  }
};

} // namespace fast
