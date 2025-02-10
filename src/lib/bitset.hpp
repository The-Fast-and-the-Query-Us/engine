#pragma once

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/_types/_ssize_t.h>
#include <sys/fcntl.h>

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

  bitset() : num_bits(0), num_int64(0), bits(nullptr), dump_file_path("") {}

  bitset(size_t _num_bits, const char *file_path = nullptr) : num_bits(_num_bits), dump_file_path(file_path) {
    if (!file_path) {
      num_int64 = uint64_from_bits(num_bits);
      bits = new uint64_t[num_int64]();
      return;
    }

    int fd = open(dump_file_path, O_RDONLY);
    
    if (fd == -1) {
      if (errno == ENOENT) { // FILE DOES NOT EXIST
        num_int64 = uint64_from_bits(num_bits);
        bits = new uint64_t[num_int64]();
        return;
      }
      std::cerr << "Failed to open bitset dump file on read\n";
    }

    ssize_t bytes_read = read(fd, &num_int64, sizeof(size_t));
    if (bytes_read == -1) {
      std::cerr << "Write in save for bitset failed (length)\n";
      close(fd);
    }
    bits = new uint64_t[num_int64]();

    bytes_read = read(fd, bits, num_int64 * sizeof(uint64_t));
    if (bytes_read == -1) {
      std::cerr << "Write in save for bitset failed\n";
      close(fd);
    }
    
    if (close(fd) == -1) {
      std::cerr << "Error closing file in bitset save()\n";
    }
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

  ~bitset() { 
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

  void save() {
    int fd = open(dump_file_path, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
      std::cerr << "Failed to open bitset dump file on write\n";
    }

    ssize_t bytes_written = write(fd, &num_int64, sizeof(size_t));
    if (bytes_written == -1) {
      std::cerr << "Write in save for bitset failed (length)\n";
      close(fd);
    }

    bytes_written = write(fd, bits, num_int64 * sizeof(uint64_t));
    if (bytes_written == -1) {
      std::cerr << "Write in save for bitset failed\n";
      close(fd);
    }
    
    if (close(fd) == -1) {
      std::cerr << "Error closing file in bitset save()\n";
    }
  }

private:
  static constexpr uint32_t BYTE_LEN = 8;

  size_t num_bits; // size in bits

  size_t num_int64; // size in uint64_t

  uint64_t *bits;

  const char *dump_file_path;

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
