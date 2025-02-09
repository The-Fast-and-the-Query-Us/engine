#pragma once

#include "static_string.hpp"
#include <cstddef>
#include <cstring>
#include <hashtable.hpp>

namespace fast {

/*
* Hashblob structure
* HEADERS
* buckets [ ] -> dict entrys
* dictentrys [ ] = size_t (posting start) + word + \0
* posts [ ] = word_count, sync_table_len, postings len
*             sync table [ ] (compressed docid, offset, pointer to postings)
*             postings [ ]
*/
class hashblob {
  size_t // headers
  magic, num_buckets, dict_end, num_tokens, // TODO : blob size
  // dynamic array
  data;

  static inline size_t dict_entry_size(const hashtable::bucket &b) {
    return sizeof(size_t) + b.word.length() + 1;
  }

  static inline size_t posting_size(const hashtable::bucket &b) {
    (void)b;
    return 0; // TODO
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

  /*
  * Configuration for hashmap
  */
  struct options {
    size_t num_buckets;
    size_t file_size;
  };

  static options get_opts(const hashtable *ht) {
    constexpr double MULT = 1; // factor for number of buckets

    size_t num_words{0}; 
    size_t dynamic{0};

    for (auto i = 0ul; i < ht->num_buckets_; ++i) {
      for (const auto &bucket : ht->buckets_[i]) {
        ++num_words;
        dynamic += dict_entry_size(bucket) + posting_size(bucket);
      }
    }
    options opts;
    opts.num_buckets = num_words * MULT;
    opts.file_size = sizeof(hashblob) + sizeof(size_t) * (num_words - 1) + dynamic;
    return opts;
  }

  static void write(const hashtable *ht, hashblob *buffer, size_t num_buckets) {
    buffer->num_buckets = num_buckets; 
    size_t *data = &buffer->data;
    
    // calculate size needed for headers
    for (auto i = 0ul; i < ht->num_buckets_; ++i) {
      for (const auto &bucket : ht->buckets_[i]) {
        data[bucket.hash_val % num_buckets] += dict_entry_size(bucket);
        ++buffer->num_tokens;
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
        memcpy(bucket_list, bucket.word.begin(), bucket.word.length());
        bucket_list[bucket.word.length()] = 0;
      }
    }

    // write posting list for each word

    buffer->magic = 42; // write magic at the end to make whole write "atomic"
  }

  size_t *buckets() { return &data; }

  char *dict() { return reinterpret_cast<char*>(&data + num_buckets); }

  char *posts() { return reinterpret_cast<char*>(&data + num_buckets) + dict_end; }

  char *find_entry(const static_string &s) {
    const auto hash_val = hashtable::hash(s);
    const auto offset = buckets()[hash_val % num_buckets];
    const auto end = (hash_val % num_buckets == num_buckets - 1) ? dict_end : buckets()[(hash_val + 1) % num_buckets];

    for (auto ptr = dict() + offset; ptr != dict() + end; ptr += sizeof(size_t) + strlen(ptr + sizeof(size_t))) {
      if (s == ptr + sizeof(size_t)) return ptr;
    }

    return nullptr;
  }

};

}


