#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fast {

class bitset {
public:
  // ============== Proxy Class ==============

  class bit_ref {
  public:
    bit_ref(uint64_t &_ele, uint64_t _mask) : ele(_ele), mask(_mask) {}

    operator bool() const { return (ele & mask) != 0; }

    bit_ref &operator=(const bool bit) {
      if (bit)
        ele |= mask;
      else
        ele &= ~mask;

      return *this;
    }

    bit_ref &operator=(const bit_ref &other) {
      return *this = static_cast<bool>(other);
    }

  private:
    uint64_t &ele, mask;
  };

  // ======== Constructor and Methods ========

  bitset(size_t _size) : size_(_size) {
    elts_size_ = (size_ + 1) / (sizeof(uint64_t) * BYTE_LEN);

    bits = new uint64_t[elts_size_];
  }

  size_t size() { return size_; }

  // =============== Operators ===============

  bit_ref operator[](const size_t idx) {
    if (idx >= size_)
      throw std::out_of_range("Index: " + std::to_string(idx) +
                              " out of range. Bit set has capacity " +
                              std::to_string(size_));

    size_t ele_idx = idx / sizeof(uint64_t);
    size_t bit_idx = idx / BYTE_LEN;
    uint64_t mask = 1UL << bit_idx;
    return bit_ref(bits[ele_idx], mask);
  }

  bool operator[](const size_t idx) const {
    if (idx >= size_)
      throw std::out_of_range("Index: " + std::to_string(idx) +
                              " out of range. Bit set has capacity " +
                              std::to_string(size_));

    size_t ele_idx = idx / sizeof(uint64_t);
    size_t bit_idx = idx / BYTE_LEN;
    uint64_t mask = 1UL << bit_idx;
    return (bits[ele_idx] & mask) != 0;
  }

private:
  static constexpr uint32_t BYTE_LEN = 8;

  size_t size_; // size in bits

  size_t elts_size_; // size in uint64_t

  uint64_t *bits;
};

} // namespace fast

int main(int argc, char *argv[]) {
  size_t n = 100;
  fast::bitset bs(n);

  for (int i = 0; i < bs.size(); ++i) {
    bs[i] = i % 2;
  }

  for (int i = 0; i < bs.size(); ++i) {
    std::cout << bs[i] << std::endl;
  }
}
