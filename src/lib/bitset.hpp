#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
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

  bitset() : bit_sz(0), byte8_sz(0) { bits = nullptr; }

  bitset(size_t _bit_sz) : bit_sz(_bit_sz) {
    byte8_sz = uint64_from_bits(bit_sz);

    bits = new uint64_t[byte8_sz];
  }

  ~bitset() { delete[] bits; }

  size_t size() { return bit_sz; }

  void resize(const size_t new_bit_sz) {
    size_t tmp_byte8_sz = uint64_from_bits(new_bit_sz);
    uint64_t *tmp = new uint64_t[tmp_byte8_sz];

    std::memcpy(tmp, bits, byte8_sz * 8);
    bit_sz = new_bit_sz;
    byte8_sz = tmp_byte8_sz;

    delete[] bits;
    bits = tmp;
  }

  // =============== Operators ===============

  bitset operator=(const bitset &rhs) {
    delete[] bits;
    std::memcpy(bits, rhs.bits, byte8_sz * 8);
    bit_sz = rhs.bit_sz;
    byte8_sz = rhs.byte8_sz;
    return *this;
  }

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

  size_t bit_sz; // size in bits

  size_t byte8_sz; // size in uint64_t

  uint64_t *bits;

  void calc_idx_info(size_t idx, size_t &ele_idx, uint64_t &mask) const {
    if (idx >= bit_sz)
      throw std::out_of_range("Index: " + std::to_string(idx) +
                              " out of range. Bit set has capacity " +
                              std::to_string(bit_sz));

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
