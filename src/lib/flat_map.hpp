#pragma once

// TODO: NEED TO TEST
#include <cassert>
#include <iostream>

#include "pair.hpp"
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cstdlib>
#include "string.hpp"
#include "common.hpp"
#include "hash.hpp"
#include <type_traits>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// ARM NEON (Apple Silicon)
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define NEON 1
// x86/x64 with SSE2
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <emmintrin.h>
#define X86_64 1
#endif

namespace fast {

template <typename K, typename V>
class flat_map {
private:
  static constexpr size_t INITIAL_SIZE = 256;
  static constexpr uint8_t EMPTY = 0x80;
  static constexpr uint8_t DELETED = 0xFE;
  static constexpr uint8_t LAST_SEVEN_BITS = 0x7F;
  static constexpr float LOAD_FACTOR_THRESHOLD = 0.875;
  static constexpr size_t MAX_URL_LEN = 8092;

  struct entry {
    K key;
    V value;
  };

  uint8_t *meta{};
  entry *data{};
  size_t size{};
  size_t cap{};

  void grow(size_t new_cap) {
    uint8_t *old_meta = meta;
    entry *old_data = data;
    size_t old_cap = cap;

    // meta = static_cast<uint8_t*>(malloc(new_cap)); // NOLINT
    meta = new uint8_t[new_cap];
    memset(meta, EMPTY, new_cap);
    // data = static_cast<entry*>(malloc(new_cap * sizeof(entry))); // NOLINT
    data = new entry[new_cap];
    // for (size_t i = 0; i < new_cap; ++i) {
    //   new (data + i) entry(K(), V());
    // }

    cap = new_cap;
    size = 0;

    for (size_t i = 0; i < old_cap; ++i) {
      if (old_meta[i] != EMPTY && old_meta[i] != DELETED) {
        // Manages size increment
        insert(old_data[i].key, old_data[i].value);
      }
    }


    // free(old_meta);
    delete[] old_meta;
    // free(old_data);
    delete[] old_data;
  }

  fast::pair<uint64_t, uint8_t> hash_key(const K &key) {
    uint64_t h{};
    h = fast::hash(key.begin());
    uint64_t data_hash = h >> 7; // NOLINT (magic num)
    auto meta_hash = static_cast<uint8_t>(h & LAST_SEVEN_BITS);
    return {data_hash, meta_hash};
  }

#if NEON
  uint16_t compare_bytes(const uint8_t* data, uint8_t pattern) {
    uint8x16_t vpattern = vdupq_n_u8(pattern);
    uint8x16_t vdata = vld1q_u8(data);
    uint8x16_t vcmp = vceqq_u8(vdata, vpattern);

    uint16_t result = 0;
    uint8_t* cmp_result = (uint8_t*)&vcmp;
    for (uint8_t i = 0; i < 16; i++) {
      if (cmp_result[i] & 0x80) {
        result |= (1 << i);
      }
    }
    return result;
  }

#elif X86_64
  uint16_t compare_bytes(const uint8_t* data, uint8_t pattern) {
    __m128i vpattern = _mm_set1_epi8(pattern);
    __m128i vdata = _mm_loadu_si128((__m128i*)data);
    __m128i vcmp = _mm_cmpeq_epi8(vdata, vpattern);

    return _mm_movemask_epi8(vcmp);
  }

#else
  uint16_t compare_bytes(const uint8_t* data, uint8_t pattern) {
    uint16_t result = 0;
    for (uint8_t i = 0; i < 16; i++) {
      if (data[i] == pattern) {
        result |= (1 << i);
      }
    }
    return result;
  }

#endif

public:
  flat_map(size_t _cap = INITIAL_SIZE) {
    grow(_cap);
  }

  flat_map(const flat_map &other) :
    size(other.size) {
    grow(other.cap);
    std::memcpy(meta, other.meta, cap);
    std::memcpy(data, other.data, cap);
  }

  flat_map &operator=(const flat_map &other) {
    if (this != &other) {
      grow(other.cap);
      size = other.size;
      std::memcpy(meta, other.meta, cap);
      std::memcpy(data, other.data, cap);
    }
    return *this;
  }

  flat_map(flat_map &&other) noexcept :
    size(other.size),
    cap(other.cap),
    meta(other.meta),
    data(other.data) {
    other.meta = nullptr;
    other.data = nullptr;
    other.size = 0;
    other.cap = 0;
  }

  flat_map &operator=(flat_map &&other) noexcept {
    if (this != &other) {
      size = other.size;
      cap = other.cap;
      meta = other.meta;
      data = other.data;
      other.meta = nullptr;
      other.data = nullptr;
      other.size = 0;
      other.cap = 0;
    }
    return *this;
  }

  ~flat_map() {
    // free(meta); // NOLINT (RAII)
    delete[] meta;
    // free(data);
    delete[] data;
  }


  V* find(const K &key) {
    fast::pair<uint64_t, uint8_t> h = hash_key(key);
    size_t chunk_start = fast::min(static_cast<size_t>(h.first) % cap, cap - 16); // NOLINT magic num
    uint32_t cmp_mask = compare_bytes(meta + chunk_start, h.second);
    for (size_t chunk = 0; chunk < cap >> 4; ++chunk) {
      while (cmp_mask) {
        int pos = __builtin_ctz(cmp_mask);
        if (data[chunk_start + pos].key == key) {
          return &data[chunk_start + pos].value;
        }
        cmp_mask &= (cmp_mask - 1);
      }
      uint8_t *ptr = meta + chunk_start;
      for (uint8_t addy = 0; addy < 16; ++ptr, ++addy) { // NOLINT magic num
        if (*ptr == EMPTY) { return &data[chunk_start + addy].value; }
      }
      chunk_start = fast::min((chunk_start + 16) % cap, cap - 16); // NOLINT magic num
      cmp_mask = compare_bytes(meta + chunk_start, h.second);
    }
    return nullptr;
  }

  bool contains(const K &key) {
    return find(key);
  }

  void insert(const K &key, const V &value) {
    if (size >= cap * LOAD_FACTOR_THRESHOLD) {
      grow(cap * 2);
    }
    fast::pair<uint64_t, uint8_t> h = hash_key(key);
    size_t chunk_start = fast::min(static_cast<size_t>(h.first) % cap, cap - 16); // NOLINT magic num
    uint32_t cmp_mask = compare_bytes(meta + chunk_start, h.second);
    entry* first_deleted{};
    size_t first_deleted_meta_index{};
    for (size_t i = 0; i < cap >> 4; ++i) {
      while (cmp_mask) {
        int pos = __builtin_ctz(cmp_mask);
        if (data[chunk_start + pos].key == key) {
          data[chunk_start + pos].value = value;
          return;
        }
        cmp_mask &= (cmp_mask - 1);
      }
      for (uint8_t i = 0; i < 16; ++i) { // NOLINT magic num
        size_t index = chunk_start + i;
        if (meta[index] == EMPTY) { 
          data[index].key = key;
          data[index].value = value;
          meta[index] = h.second;
          ++size;
          return;
        }
        if (meta[index] == DELETED && first_deleted == nullptr) {
          first_deleted = &data[index];
          first_deleted_meta_index = index;
        }
      }
      if (first_deleted) {
        first_deleted->key = key;
        first_deleted->value = value;
        meta[first_deleted_meta_index] = h.second;
        ++size;
        return;
      }
      chunk_start = fast::min((chunk_start + 16) % cap, cap - 16); // NOLINT magic num
      cmp_mask = compare_bytes(meta + chunk_start, h.second);
    }
  }

  V& operator[](const K &key) {
    if (size >= cap * LOAD_FACTOR_THRESHOLD) {
      grow(cap * 2);
    }
    fast::pair<uint64_t, uint8_t> h = hash_key(key);
    size_t chunk_start = fast::min(static_cast<size_t>(h.first) % cap, cap - 16); // NOLINT magic num
    uint32_t cmp_mask = compare_bytes(meta + chunk_start, h.second);
    entry* first_deleted{};
    size_t first_deleted_meta_index{};
    for (size_t i = 0; i < cap >> 4; ++i) {
      while (cmp_mask) {
        int pos = __builtin_ctz(cmp_mask);
        if (data[chunk_start + pos].key == key) {
          return data[chunk_start + pos].value;
        }
        cmp_mask &= (cmp_mask - 1);
      }
      for (size_t i = 0; i < 16; ++i) { // NOLINT magic num
        size_t index = chunk_start + i;
        if (meta[index] == EMPTY) { 
          data[index].key = key;
          data[index].value = V();
          meta[index] = h.second;
          ++size;
          return data[index].value;
        }
        if (meta[index] == DELETED && !first_deleted) {
          first_deleted = &data[index];
          first_deleted_meta_index = index;
        }
      }
      if (first_deleted) {
        first_deleted->key = key;
        first_deleted->value = V();
        meta[first_deleted_meta_index] = h.second;
        ++size;
        return first_deleted->value;
      }
      chunk_start = fast::min((chunk_start + 16) % cap, cap - 16); // NOLINT magic num
      cmp_mask = compare_bytes(meta + chunk_start, h.second);
    }
    return data[0].value;
  }

  const V& operator[](const K &key) const {
    auto found = find(key);
    return found;
  }

  void remove(const K &key) {
    fast::pair<uint64_t, uint8_t> h = hash_key(key);
    size_t chunk_start = fast::min(static_cast<size_t>(h.first) % cap, cap - 16); // NOLINT magic num
    uint32_t cmp_mask = compare_bytes(meta + chunk_start, h.second);
    for (size_t i = 0; i < cap >> 4; ++i) {
      while (cmp_mask) {
        int pos = __builtin_ctz(cmp_mask);
        if (data[chunk_start + pos].key == key) {
          meta[chunk_start + pos] = DELETED;
          --size;
          return;
        }
        cmp_mask &= (cmp_mask - 1);
      }
      for (uint8_t i = 0; i < 16; ++i) { // NOLINT magic num
        size_t index = chunk_start + i;
        if (meta[index] == EMPTY) { 
          return;
        }
      }
      chunk_start = fast::min((chunk_start + 16) % cap, cap - 16); // NOLINT magic num
      cmp_mask = compare_bytes(meta + chunk_start, h.second);
    }
  }

  void erase() {
    std::memset(meta, EMPTY, cap);
    for (size_t i = 0; i < cap; ++i) {
      data[i] = entry{};
    }
    size = 0;
  }

  bool empty() {
    return !size;
  }

  size_t count() {
    return size;
  }

  bool save(int fd) {
    if (write(fd, &cap, sizeof(cap)) != sizeof(cap)) {
      return false;
    }
    if (write(fd, &size, sizeof(size)) != sizeof(size)) { // Fixed: was sizeof(cap)
      return false;
    }
    if (write(fd, meta, cap) != static_cast<ssize_t>(cap)) {
      return false;
    }

    for (size_t i = 0; i < cap; ++i) {
      if (meta[i] != EMPTY && meta[i] != DELETED) {
        size_t key_size = strlen(data[i].key.begin()) + 1;
        if (write(fd, &key_size, sizeof(key_size)) != sizeof(key_size)) {
          return false;
        }
        if (key_size > 0 && 
          write(fd, data[i].key.begin(), key_size) != static_cast<ssize_t>(key_size)) {
          return false;
        }

        auto value_size = sizeof(data[i].value);
        if (write(fd, &value_size, sizeof(value_size)) != sizeof(value_size)) {
          return false;
        }
        if (value_size > 0 && 
          write(fd, &data[i].value, value_size) != static_cast<ssize_t>(value_size)) {
          return false;
        }
      }
    }

    return true;
  }

  bool load(int fd) {
    size_t temp_cap{};
    if (read(fd, &temp_cap, sizeof(temp_cap)) != sizeof(cap)) {
      return false;
    }
    grow(temp_cap);

    if (read(fd, &size, sizeof(size)) != sizeof(size)) {
      return false;
    }
    if (read(fd, meta, cap) != static_cast<ssize_t>(cap)) {
      return false;
    }


    for (size_t i = 0; i < cap; ++i) {
      if (meta[i] != EMPTY && meta[i] != DELETED) {
        size_t kv_size{};
        char buf[MAX_URL_LEN];

        if (read(fd, &kv_size, sizeof(kv_size)) != sizeof(kv_size)) {
          return false;
        }
        if (kv_size > 0 && 
          read(fd, &buf, kv_size) != static_cast<ssize_t>(kv_size)) { // We include the null character
          return false;
        }

        for (size_t idx = 0; idx < kv_size; ++idx) {
          data[i].key += buf[idx];
        }

        if (read(fd, &kv_size, sizeof(kv_size)) != sizeof(kv_size)) {
          return false;
        }
        if (kv_size > 0 && 
          read(fd, &data[i].value, kv_size) != static_cast<ssize_t>(kv_size)) {
          return false;
        }
      }
    }

    return true;
  }

  void print() {
    for (size_t i = 0; i < cap; ++i) {
      if (meta[i] != EMPTY && meta[i] != DELETED) {
        std::cout << "key: " << data[i].key.begin() << '\n'
          << "val: " << data[i].value << '\n'
          << "meta: " << meta[i] << '\n';
      }
    }
  }
};
}
