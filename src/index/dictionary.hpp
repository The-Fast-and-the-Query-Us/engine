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
  static constexpr double LOAD = 1.0; // multiple of tokens for num_buckets

  size_t num_buckets, num_unique, num_words, dict_size;

  size_t *buckets() { return &dict_size + 1; }
  const size_t *buckets() const { return &dict_size + 1; }

  unsigned char *dict() {
    return reinterpret_cast<unsigned char*>(buckets() + num_buckets);
  }

  const unsigned char *dict() const {
    return reinterpret_cast<const unsigned char*>(buckets() + num_buckets);
  }

  public:


  size_t unique() const { return num_unique; }
  size_t words()  const { return num_words; }

  static size_t size_needed(const hashtable &ht) {
    size_t dynamic{0};
    
    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        dynamic += sizeof(size_t) + bucket.word.size() + 1;
      }
    }

    dynamic += ht.unique_words * LOAD * sizeof(size_t) + sizeof(dictionary);

    return dynamic;
  }

  /*
   * buffer must be zero init and align of size_t.
   * constructs dictionary in buffer with each key set to dict[key] = 1
   */
  static unsigned char *write(const hashtable &ht, dictionary *buffer) {
    buffer->num_unique = ht.unique_words;
    buffer->num_buckets = ht.unique_words * LOAD;
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

  const unsigned char *find_entry(const string_view &word) const {
    const auto hash_val = hash(word);
    auto pos = buckets()[hash_val % num_buckets] + dict();
    const auto end = dict() + (((hash_val + 1) % num_buckets) ?
                     buckets()[(hash_val + 1) % num_buckets] : dict_size);

    while (pos != end) {
      auto key = pos + sizeof(size_t);
      size_t len;

      for (len = 0; len < word.size() && word[len] == *key; ++len, ++key);

      if (len == word.size() && *key == 0) return pos;

      pos = key + len;
      while (*(pos++));
    }

    return nullptr;
  }

  unsigned char *find_entry(const string_view &word) {
    return const_cast<unsigned char*>(static_cast<const dictionary*>(this)->find_entry(word));
  }

  // requires that word is in the dict
  void put(const string_view &word, size_t val) {
    auto entry = find_entry(word);
    write_unaligned(val, entry);
  }

  // returns pair {value, bool (true => present in map)}
  pair<size_t, bool> get(const string_view &word) const {
    auto entry = find_entry(word);
    if (entry == nullptr) return {0, false};
    return {read_unaligned<size_t>(entry), true};
  }

};

}
