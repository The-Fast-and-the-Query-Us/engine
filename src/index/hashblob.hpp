#pragma once

#include <cstddef>
#include <cstring>
#include <static_string.hpp>
#include <hashtable.hpp>
#include <postlist.hpp>

namespace fast {

/*
* Hashblob structure
* HEADERS
* buckets [ ] -> dict entrys
* dictentrys [ ] = size_t (posting start) + word + \0
*/

class hashblob {
  static constexpr double TOKEN_MULT = 1.0; // factor for number of buckets

  size_t // headers
  magic, num_buckets, dict_len,
  // dynamic array
  data;

  static inline size_t dict_entry_size(const hashtable::bucket &b) {
    return sizeof(size_t) + b.word.length() + 1;
  }

  template <class T> // move to lib
  static inline void read_unaligned(T &val, char *buffer) {
    memcpy(&val, buffer, sizeof(T));
  }

  template <class T> // move to lib
  static inline void write_unaligned(const T &val, char *buffer) {
    memcpy(buffer, &val, sizeof(T));
  }

  // data access
  size_t *buckets() {
    return &data;
  }

  char *dict_start() {
    return reinterpret_cast<char*>(&data + num_buckets);
  }

  char *dict_end() {
    return dict_start() + dict_len;
  }

  // return a pointer to the size_t that is offset for the words posting list
  char *get_dict_entry(const static_string &ss) {
    const auto hashVal = hashtable::hash(ss);
    auto start = dict_start() + buckets()[hashVal % num_buckets];
    const auto end = ((hashVal + 1) % num_buckets == 0) ? dict_end() : dict_start() + buckets()[(hashVal + 1) % num_buckets];

    while (start < end) {
      if (ss == start + sizeof(size_t)) return start;
      start += sizeof(size_t);
      while (*(start++));
    }

    return nullptr;
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
    size_t num_words{0}; 
    size_t dynamic{0};

    for (const auto &l : *ht) {
      for (const auto &bucket : l) {
        ++num_words;
        dynamic += dict_entry_size(bucket) + postlist::size_needed(bucket.posts);
      }
    }

    options opts;
    opts.num_buckets = num_words * TOKEN_MULT;
    opts.file_size = sizeof(hashblob) + sizeof(size_t) * (opts.num_buckets - 1) + dynamic;
    return opts;
  }

  static void write(const hashtable *ht, hashblob *buffer, const options *opts) {
    write_dict(ht, buffer, opts);
    write_posts(ht, buffer);
    buffer->magic = 42;
  }

  // buffer must be zero init
  static void write_dict(const hashtable *ht, hashblob *buffer, const options *opts) {
    buffer->num_buckets = opts->num_buckets;

    for (const auto &bucketList : *ht) {
      for (const auto &bucket : bucketList) {
        buffer->buckets()[bucket.hash_val % buffer->num_buckets] += dict_entry_size(bucket);
      }
    }

    auto accumulator = 0ul;
    for (auto i = 0ul; i < buffer->num_buckets; ++i) {
      accumulator += buffer->buckets()[i];
      buffer->buckets()[i] = accumulator - buffer->buckets()[i];
    }

    buffer->dict_len = accumulator;

    size_t reader;
    for (const auto &bucketList : *ht) {
      for (const auto &bucket : bucketList) {
        auto start = buffer->dict_start() + buffer->buckets()[bucket.hash_val % buffer->num_buckets];

        for (read_unaligned(reader, start); reader ;read_unaligned(reader, start)) {
          start += sizeof(size_t);
          while (*(start++));
        }

        reader = 1;
        write_unaligned(reader, start);
        memcpy(start + sizeof(size_t), bucket.word.begin(), bucket.word.length());
        start[sizeof(size_t) + bucket.word.length()] = 0;
      }
    }
  }

  static void write_posts(const hashtable *ht, hashblob *buffer) {
    auto writePos = buffer->dict_end();

    for (const auto &bucketList : *ht) {
      for (const auto &bucket : bucketList) {
        size_t offset = writePos - buffer->dict_end();
        write_unaligned(offset, buffer->get_dict_entry(bucket.word));
        writePos = postlist::write(bucket.posts, writePos);
      }
    }
  }
  
  // reuse earlier function
  char *get_posts(const static_string &ss) {
    const auto hashVal = hashtable::hash(ss);
    auto start = dict_start() + buckets()[hashVal % num_buckets];
    const auto end = ((hashVal + 1) % num_buckets == 0) ? dict_end() : dict_start() + buckets()[(hashVal + 1) % num_buckets];

    size_t reader;
    while (start < end) {
      if (ss == start + sizeof(size_t)) {
        read_unaligned(reader, start);
        return dict_end() + reader;
      }

      start += sizeof(size_t);
      while (*(start++));
    }
    return nullptr;
  }

};

}


