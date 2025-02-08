#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

namespace fast {

class bitset {
public:
  // ================ Proxy Class ================

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

    bool operator!() const { return (ele & mask) == 0; }

    bool operator==(const bool rhs) const {
      return static_cast<bool>(*this) == rhs;
    }

    bool operator!=(const bool rhs) const { return !(*this == rhs); }

    bool operator==(const int rhs) const {
      return *this == static_cast<bool>(rhs);
    }

    bool operator!=(const int rhs) const { return !(*this == rhs); }

  private:
    uint64_t &ele, mask;
  };

  // ========== Constructor and Methods ==========

  bitset() : num_bits(0), num_int64(0) { bits = nullptr; }

  bitset(size_t _num_bits) : num_bits(_num_bits) {
    num_int64 = uint64_from_bits(num_bits);

    bits = new uint64_t[num_int64]();
  }

  bitset(const bitset &rhs) : num_bits(rhs.num_bits), num_int64(rhs.num_int64) {
    bits = new uint64_t[num_int64];
    std::memcpy(bits, rhs.bits, num_int64 * sizeof(uint64_t));
  }

  bitset &operator=(const bitset &rhs) {
    if (this != &rhs) {
      delete[] bits;
      num_bits = rhs.num_bits;
      num_int64 = rhs.num_int64;
      bits = new uint64_t[num_int64];
      std::memcpy(bits, rhs.bits, num_int64 * sizeof(uint64_t));
    }
    return *this;
  }

  ~bitset() { delete[] bits; }

  inline size_t size() { return num_bits; }

  void resize(const size_t new_num_bits) {
    size_t new_num_int64 = uint64_from_bits(new_num_bits);
    uint64_t *tmp = new uint64_t[new_num_int64];

    uint64_t copy_sz = num_int64 < new_num_int64 ? num_int64 : new_num_int64;
    std::memcpy(tmp, bits, copy_sz * sizeof(uint64_t));

    num_bits = new_num_bits;
    num_int64 = new_num_int64;

    delete[] bits;
    bits = tmp;
  }

  // ================= Operators =================

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

  size_t num_bits; // size in bits

  size_t num_int64; // size in uint64_t

  uint64_t *bits;

  // ================== Helpers =================
  void calc_idx_info(size_t idx, size_t &ele_idx, uint64_t &mask) const {
    if (idx >= num_bits)
      throw std::out_of_range("Index: " + std::to_string(idx) +
                              " out of range. Bit set has capacity " +
                              std::to_string(num_bits));

    ele_idx = idx / (sizeof(uint64_t) * BYTE_LEN);
    size_t bit_idx = idx % (sizeof(uint64_t) * BYTE_LEN);
    mask = 1UL << bit_idx;
  }

  size_t uint64_from_bits(size_t num_bits) {
    return (num_bits + (sizeof(uint64_t) * BYTE_LEN - 1)) /
           (sizeof(uint64_t) * BYTE_LEN);
  }
};

} // namespace fast
