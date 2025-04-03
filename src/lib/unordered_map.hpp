#pragma once

#include <cstdint>
#include <cstdlib>
#include <hash.hpp>
#include <common.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

namespace fast {

template<typename K, typename V>
class unordered_map {

  static constexpr uint8_t INITIAL_CAPACITY = 128;
  static constexpr float LOAD_FACTOR = 0.7;
  static constexpr uint16_t MAX_URL_LEN = 8096;

  enum func_type : uint8_t {
    CONTAINS = 0,
    OPERATOR = 1,
    REMOVE = 2
  };

  enum b_status : uint8_t {
    EMPTY = 0,
    FULL = 1,
    DELETED = 2
  };

  size_t cap{};
  size_t size{};

  struct bucket {
    K key;
    V value;
    b_status status;
  };

  bucket *buckets{};

  void grow(size_t new_cap) {
    if (!new_cap) { new_cap = fast::max((size_t)INITIAL_CAPACITY, cap); };
    bucket *old_buckets = buckets;
    size_t old_cap = cap;

    buckets = static_cast<bucket*>(malloc(new_cap * sizeof(bucket))); // NOLINT
    // buckets = new (memory) bucket{K{}, V{}, EMPTY};
    for (size_t i = 0; i < new_cap; ++i) {
      new (&buckets[i]) bucket{K{}, V{}, EMPTY};
    }

    cap = new_cap;
    if (!size) { 
      if (old_buckets) { free(old_buckets); }
      return;
    }

    size = 0;
    for (size_t i = 0; i < old_cap; ++i) {
      if (old_buckets[i].status == FULL) {
        (*this)[old_buckets[i].key] = old_buckets[i].value;
      }
    }

    if (old_buckets) { free(old_buckets); }
  }

  

public:
  unordered_map(size_t size_ = INITIAL_CAPACITY) {
    if (size_ % 2) { ++size_; }
    grow(size_);
  }

  ~unordered_map() {
    if (buckets) { free(buckets); }
  }
  
  bool contains(const K &key) {
    if (!cap) { return false; }
    uint64_t hash_val = hash(key.begin());
    size_t start_pos = hash_val % cap;
    size_t i{1};
    
    if (buckets[start_pos].status == FULL && buckets[start_pos].key == key) {
      return true;
    }

    while (true) {
      size_t probe = (start_pos + i * i) % cap;
      if (buckets[probe].status == EMPTY) {
        return false;
      }

      if (buckets[probe].status == FULL && buckets[probe].key == key) {
        return true;
      }

      ++i;

      if (i >= cap || probe == start_pos) {
        return false;
      }
    }
  }

  void remove(const K &key) {
    uint64_t hash_val = hash(key.begin());
    size_t start_pos = hash_val % cap;
    size_t i{1};
    
    if (buckets[start_pos].status == FULL && buckets[start_pos].key == key) {
      buckets[start_pos].status = DELETED;
      --size;
      return;
    }

    while (true) {
      size_t probe = (start_pos + i * i) % cap;
      if (buckets[probe].status == EMPTY) {
        return;
      }

      if (buckets[probe].status == FULL && buckets[probe].key == key) {
        buckets[probe].status = DELETED;
        --size;
        return;
      }

      ++i;

      if (i >= cap || probe == start_pos) {
        return;
      }
    }
  }

  V& operator[](const K &key) {
    if (!cap || size > cap * LOAD_FACTOR) {
      grow(!cap ? INITIAL_CAPACITY : cap * 2);
    }

    uint64_t hash_val = hash(key.begin());
    size_t start_pos = hash_val % cap;
    size_t i{1};
    int64_t first_deleted = -1;
    
    if (buckets[start_pos].status == FULL && buckets[start_pos].key == key) {
      return buckets[start_pos].value;
    }

    while (true) {
      size_t probe = (start_pos + i * i) % cap;
      if (buckets[probe].status == EMPTY) {
        size_t insert_pos = (first_deleted == -1 ? probe : first_deleted);
        buckets[insert_pos] = {key, V{}, FULL};
        ++size;
        return buckets[insert_pos].value;
      }

      if (buckets[probe].status == FULL && buckets[probe].key == key) {
        return buckets[probe].value;
      }

      if (buckets[probe].status == DELETED && first_deleted == -1) {
        first_deleted = probe;
      }

      ++i;

      if (i >= cap || probe == start_pos) {
        grow(cap * 2);
        return (*this)[key];
      }
    }
  }

  size_t count() {
    return size;
  }

  void erase() {
    for (size_t i = 0; i < cap; ++i) {
      buckets[i] = {K{}, V{}, EMPTY};
    }
    size = 0;
  }

  bool save(int fd) {
    if (write(fd, &cap, sizeof(cap)) != sizeof(cap)) {
      return false;
    }
    if (write(fd, &size, sizeof(size)) != sizeof(size)) {
      return false;
    }

    for (size_t i = 0; i < cap; ++i) {
      if (buckets[i].status == FULL) {
        size_t key_size = buckets[i].key.size();
        auto value_size = sizeof(buckets[i].value);
        if (write(fd, &key_size, sizeof(key_size)) != sizeof(key_size)) {
          return false;
        }
        if (write(fd, &value_size, sizeof(value_size)) != sizeof(value_size)) {
          return false;
        }

        if (key_size > 0 && 
          write(fd, buckets[i].key.begin(), key_size) != static_cast<ssize_t>(key_size)) {
          return false;
        }
        if (value_size > 0 && 
          write(fd, &buckets[i].value, value_size) != static_cast<ssize_t>(value_size)) {
          return false;
        }
      }
    }

    return true;
  }

  bool load(int fd) {
    erase();

    size_t temp_cap{};
    if (read(fd, &temp_cap, sizeof(cap)) != sizeof(cap)) {
      return false;
    }

    size_t map_size{};
    if (read(fd, &map_size, sizeof(map_size)) != sizeof(map_size)) {
      return false;
    }

    if (!temp_cap) { temp_cap = INITIAL_CAPACITY; }
    grow(temp_cap);

    for (size_t i = 0; i < map_size; ++i) {
      size_t key_size{};
      size_t value_size{};
      if (read(fd, &key_size, sizeof(key_size)) != sizeof(key_size)) {
        return false;
      }
      if (read(fd, &value_size, sizeof(value_size)) != sizeof(value_size)) {
        return false;
      }

      if (key_size >= MAX_URL_LEN || !key_size) {
        continue;
      }

      char key_buffer[MAX_URL_LEN + 1];
      memset(key_buffer, 0, sizeof(key_buffer));
      if (key_size > 0 && 
        read(fd, key_buffer, key_size) != static_cast<ssize_t>(key_size)) {
        return false;
      }
      K key(key_buffer, key_size);

      V val{};
      if (value_size > 0 && 
        read(fd, &val, value_size) != static_cast<ssize_t>(value_size)) {
        return false;
      }

      (*this)[key] = val;
    }

    return true;
  }
};

}
