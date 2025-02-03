#include <cmath>
#include <concepts>
#include <openssl/md5.h>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T>
concept Primitive = std::is_arithmetic_v<T>;

template <typename T>
concept Complex = requires(T v) {
  { v.data() } -> std::convertible_to<const void *>;
  { v.size() } -> std::convertible_to<size_t>;
};

template <typename T>
concept Hashable = Primitive<T> || Complex<T>;

template <Hashable T>
class BloomFilter {
public:
  BloomFilter(size_t _n, double _fpr) : n(_n), fpr(_fpr) {
    const double log2 = std::log(2);
    num_bits = (-1 * n * std::log(fpr)) / (std::pow(log2, 2));
    bit_set.resize(num_bits);

    num_hash = (num_bits / n) * log2;
  }

  void insert(const T &val) {
    auto [h1, h2] = hash(val);
    for (int i = 0; i < num_hash; ++i) {
      bit_set[double_hash(h1, h2, i)] = 1;
    }
  }

  bool contains(const T &val) {
    auto [h1, h2] = hash(val);
    for (int i = 0; i < num_hash; ++i) {
      if (!bit_set[double_hash(h1, h2, i)])
        return false;
    }
    return true;
  }

private:
  size_t n;

  uint64_t num_bits;

  double fpr;

  uint64_t num_hash;

  std::vector<bool> bit_set;

  std::pair<unsigned char *, size_t> serialize(const T &datum) {
    if constexpr (Complex<T>)
      return {reinterpret_cast<unsigned char *>(datum.data()), datum.size()};
    else if constexpr (Primitive<T>)
      return {reinterpret_cast<unsigned char *>(&datum), sizeof(datum)};
    else
      throw std::runtime_error("The type provided cannot be hashed.");
  }

  std::size_t double_hash(uint64_t h1, uint64_t h2, uint32_t i) {
    return (h1 + i * h2) % num_bits;
  }

  std::pair<uint64_t, uint64_t> hash(const T &datum) {
    auto [serialized, len] = serialize(datum);

    unsigned char curr_hash[MD5_DIGEST_LENGTH];
    MD5(serialized, len, curr_hash);

    uint64_t h1, h2;
    std::memcpy(&h1, curr_hash, sizeof(uint64_t)); // First 8 bytes
    std::memcpy(&h2, curr_hash + sizeof(uint64_t),
                sizeof(uint64_t)); // Next 8 bytes

    return {h1, h2};
  }
};
