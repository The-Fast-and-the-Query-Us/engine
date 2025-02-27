#pragma once

#include "common.hpp"
#include <cstddef>
#include <hashtable.hpp>
#include <hash.hpp>

namespace fast {

/*
* Serialized hashmap dictionary of string to size_t
* used to map string to offset to postings list
*/
class dictionary {
  static constexpr double LOAD = 1;

  size_t num_buckets, num_unique, num_words, dict_size;

  size_t *buckets() { return &dict_size + 1; }

  char *dict() {
    return reinterpret_cast<char*>(buckets() + num_buckets);
  }

  public:

  struct options {
    size_t size_needed;
    size_t num_unique;
  };
  
  static options get_opts(const hashtable &ht) {
    options opts{0, 0};
    
    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        ++opts.num_unique;
        opts.size_needed += sizeof(size_t) + bucket.word.size() + 1;
      }
    }

    opts.size_needed += opts.num_unique * LOAD * sizeof(size_t) + sizeof(dictionary);

    return opts;
  }

  // Buffer should be zero init
  static char *write(const hashtable &ht, dictionary *buffer, options opts) {
    buffer->num_unique = opts.num_unique;
    buffer->num_buckets = opts.num_unique * LOAD;
    buffer->num_words = ht.next_offset;

    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        buffer->buckets()[bucket.hashval % buffer->num_buckets] += sizeof(size_t) + bucket.word.size() + 1;
      }
    }

    size_t accumulator = 0;
    for (auto i = 0u; i < buffer->num_buckets; ++i) {
      accumulator += buffer->buckets()[i];
      buffer->buckets()[i] = accumulator - buffer->buckets()[i];
    }
    buffer->dict_size = accumulator;

    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        const auto hashval = hash(bucket.word.c_str());

        auto write_pos = buffer->dict() + buffer->buckets()[hashval % buffer->num_buckets];

        while (read_unaligned<size_t>(write_pos) != 0) {
          write_pos += sizeof(size_t);
          while (*(write_pos++));
        }

        write_unaligned(1, write_pos);
        memcpy(write_pos + sizeof(size_t), bucket.word.c_str(), bucket.word.size() + 1);
      }
    }

    return buffer->dict() + buffer->dict_size;
  }

  /*
  * Proxy class to modify or get misaligned size_t
  */
  class val_proxy {
    char *buf;
  public:
    val_proxy(char *b) : buf(b) {}

    val_proxy& operator=(size_t st) {
      write_unaligned(st, buf);
      return *this;
    }

    operator bool() {
      return buf;
    }

    operator size_t() {
      return read_unaligned<size_t>(buf);
    }
  };

  // returns pointer to entry val if it exists or nullptr otherwise
  char* get(const string &word) {
    const auto hash_val = hash(word.c_str());

    auto pos = buckets()[hash_val % num_buckets] + dict();
    const auto end = dict() + (((hash_val + 1) % num_buckets) ? buckets()[(hash_val + 1) % num_buckets] : dict_size);

    while (pos != end) {// is strcmp faster here? since we iterate twice
      auto key = pos + sizeof(size_t);
      auto cmp = word.begin();

      for (; *key && *key == *cmp; ++key, ++cmp);
      
      if (*key == *cmp) return pos;

      while(*(key++));
      pos = key;
    }

    return nullptr;
  }

  val_proxy operator[](const string &word) {
    return get(word);
  }

};

}
