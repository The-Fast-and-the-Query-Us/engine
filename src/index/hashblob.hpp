#pragma once

#include <cstddef>
#include <cstring>
#include <hashtable.hpp>

namespace fast {

/*
* Hashblob structure
* HEADERS
* buckets [ ] -> dict entrys
* dictentrys[ ] = size_t (posting start) + word + \0
*/
class hashblob {
  size_t magic, num_buckets, dict_end, // headers
  data; // dynamic array

  static inline size_t dict_entry_size(const hashtable::bucket &b) {
    return sizeof(size_t) + b.word.length() + 1;
  }

  template <class T>
  static inline void read_unaligned(T &val, char *buffer) {
    memcpy(&val, buffer, sizeof(T));
  }

  template <class T>
  static inline void write_unaligned(const T &val, char *buffer) {
    memcpy(buffer, &val, sizeof(T));
  }

  public:

  static void write(const hashtable *ht, hashblob *buffer, size_t num_buckets) {
    buffer->num_buckets = num_buckets; 
    size_t *data = &buffer->data;
    
    // calculate size needed for headers
    for (auto i = 0ul; i < ht->num_buckets_; ++i) {
      for (const auto &bucket : ht->buckets_[i]) {
        data[bucket.hash_val % num_buckets] += dict_entry_size(bucket);
      }
    }
    
    // calc dict entry list starts (and end)
    size_t acc = 0;
    for (auto i = 0ul; i < num_buckets; ++i) {
      acc += data[i];
      data[i] = acc - data[i];
    }
    buffer->dict_end = acc;

    // write words into buckets
    char * const dict_start = reinterpret_cast<char*>(data + num_buckets);
    size_t pos;
    for (auto i = 0ul; i < ht->num_buckets_; ++i) {
      for (const auto &bucket : ht->buckets_[i]) {
        const auto offset = data[bucket.hash_val % num_buckets];
        auto bucket_list = dict_start + offset;

        // find open slot in bucket
        for (read_unaligned(pos, bucket_list); pos != 0; read_unaligned(pos, bucket_list)) {
          bucket_list += sizeof(size_t);
          bucket_list += strlen(bucket_list) + 1;
        }

        pos = 1; // mark slot as taken
        write_unaligned(pos, bucket_list);
        bucket_list += sizeof(size_t);
        memcpy(bucket_list, bucket.word.buffer_, bucket.word.length());
        bucket_list[bucket.word.length()] = 0;
      }
    }
  }
};

}


