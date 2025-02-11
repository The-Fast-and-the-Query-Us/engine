#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/_types/_ssize_t.h>
#include <sys/fcntl.h>
#include <unistd.h>

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

  bitset() : num_bits(0), num_int64(0), bits(nullptr), save_path(nullptr) {}

  bitset(size_t _num_bits, const char *_save_path = nullptr)
      : num_bits(_num_bits), save_path(_save_path) {
    num_int64 = uint64_from_bits(num_bits);
    bits = new uint64_t[num_int64]();
  }

  bitset(const char *load_path) {
    int fd = open(load_path, O_RDONLY);
    save_path = load_path;
    load(fd);
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

  int save() {
    int fd = open(save_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
      std::cerr << "Failed to open bitset dump file on write\n";
      return -1;
    }

    ssize_t int64_written = write(fd, &num_int64, sizeof(uint64_t));
    if (int64_written == -1) {
      std::cerr << "Write in save for bitset failed on member num_int64\n";
      close(fd);
      return -1;
    }

    ssize_t num_bits_written = write(fd, &num_bits, sizeof(uint64_t));
    if (num_bits_written == -1) {
      std::cerr << "Write in save for bitset failed on member num_bits\n";
      close(fd);
      return -1;
    }

    ssize_t elts_written = write(fd, bits, num_int64 * sizeof(uint64_t));
    if (elts_written == -1) {
      std::cerr << "Write in save for bitset failed on member bits\n";
      close(fd);
      return -1;
    }

    if (close(fd) == -1) {
      std::cerr << "Error closing file in bitset save()\n";
    }

    return int64_written + num_bits_written + elts_written;
  }

  int load(int fd) {
    if (fd == -1)
      throw std::runtime_error("Failed to open bitset dump file on read");

    ssize_t int64_read = read(fd, &num_int64, sizeof(uint64_t));
    if (int64_read == -1) {
      close(fd);
      throw std::runtime_error("Read in load for bitset failed (length)");
    }

    ssize_t num_bits_read = read(fd, &num_bits, sizeof(uint64_t));
    if (num_bits_read == -1) {
      close(fd);
      throw std::runtime_error("Read in load for bitset failed (length)");
    }

    delete[] bits;
    bits = new uint64_t[num_int64]();

    ssize_t elts_read = read(fd, bits, num_int64 * sizeof(uint64_t));
    if (elts_read == -1) {
      close(fd);
      throw std::runtime_error("Read in load for bitset failed\n");
    }

    if (close(fd) == -1) {
      throw std::runtime_error("Error closing file in bitset load()\n");
    }

    return int64_read + num_bits_read + elts_read;
  }

  ~bitset() {
    if (save_path)
      save();
    delete[] bits;
  }

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
  template <typename T>
  friend class bloom_filter;

  static constexpr uint32_t BYTE_LEN = 8;

  size_t num_bits; // size in bits

  size_t num_int64; // size in uint64_t

  uint64_t *bits;

  const char *save_path;

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
