#pragma once

#include <cstddef>
#include <cstdint>
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

    bool operator==(const bit_ref &rhs) {
      return (ele & mask) == (rhs.ele & rhs.mask);
    }

    bool operator==(const bool rhs) { return static_cast<bool>(*this) == rhs; }

    bool operator!=(const bit_ref &rhs) { return !(*this == rhs); }

    bool operator!=(const bool rhs) { return !(*this == rhs); }

  private:
    uint64_t &ele, mask;
  };

  // ======== Constructor and Methods ========

  bitset(size_t _size) : size_(_size) {
    elts_size_ = (size_ + (sizeof(uint64_t) * BYTE_LEN - 1)) /
                 (sizeof(uint64_t) * BYTE_LEN);

    bits = new uint64_t[elts_size_];
  }

  ~bitset() { delete[] bits; }

  size_t size() { return size_; }

  // =============== Operators ===============

  bit_ref operator[](const size_t idx) {
    size_t ele_idx;
    uint64_t mask;
    calc_idx_info(idx, ele_idx, mask);

    return bit_ref(bits[ele_idx], mask);
  }

  bool operator[](const size_t idx) const {
    size_t ele_idx;
    uint64_t mask;
    calc_idx_info(idx, ele_idx, mask);

    return (bits[ele_idx] & mask) != 0;
  }

private:
  static constexpr uint32_t BYTE_LEN = 8;

  size_t size_; // size in bits

  size_t elts_size_; // size in uint64_t

  uint64_t *bits;

  void calc_idx_info(size_t idx, size_t &ele_idx, uint64_t &mask) const {
    if (idx >= size_)
      throw std::out_of_range("Index: " + std::to_string(idx) +
                              " out of range. Bit set has capacity " +
                              std::to_string(size_));

    ele_idx = idx / (sizeof(uint64_t) * BYTE_LEN);
    size_t bit_idx = idx % (sizeof(uint64_t) * BYTE_LEN);
    mask = 1UL << bit_idx;
  }
};

} // namespace fast
