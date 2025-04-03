#pragma once

#include "common.hpp"
#include <cstddef>
#include <hashtable.hpp>
#include <hash.hpp>
#include <iostream>
#include <compress.hpp>

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

  char *dict() {
    return reinterpret_cast<char*>(buckets() + num_buckets);
  }

  const char *dict() const {
    return reinterpret_cast<const char*>(buckets() + num_buckets);
  }

  static void init_buckets(const hashtable &ht, dictionary *buffer) {
    for (auto i = 0u; i < buffer->num_buckets; ++i) {
      buffer->buckets()[i] = 0; // zero init bucket offsets
    }

    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        buffer->buckets()[bucket.hashval % buffer->num_buckets] += 
          sizeof(size_t) + bucket.word.size() + encoded_size(bucket.word.size());
      }
    }

    size_t accumulator = 0;
    for (auto i = 0u; i < buffer->num_buckets; ++i) {
      accumulator += buffer->buckets()[i];
      buffer->buckets()[i] = accumulator - buffer->buckets()[i];
    }
    buffer->dict_size = accumulator;
  }

  public:


  size_t unique() const { return num_unique; }
  size_t words()  const { return num_words; }

  static size_t size_needed(const hashtable &ht) {
    size_t dynamic{0};
    
    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        dynamic += sizeof(size_t) + bucket.word.size() + encoded_size(bucket.word.size());
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

    init_buckets(ht, buffer);

    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        const auto hashval = hash(bucket.word.c_str());

        auto &offset = buffer->buckets()[hashval % buffer->num_buckets];
        offset += sizeof(size_t);

        encode(bucket.word.size(), (unsigned char*) buffer->dict() + offset);
        offset += encoded_size(bucket.word.size());

        memcpy(buffer->dict() + offset, bucket.word.c_str(), bucket.word.size());
      }
    }

    init_buckets(ht, buffer);
    return (unsigned char *) buffer->dict() + buffer->dict_size;
  }

  const char *find_entry(const string_view &word) const {
    const auto hash_val = hash(word);
    auto pos = buckets()[hash_val % num_buckets] + dict();
    const auto end = dict() + (((hash_val + 1) % num_buckets) ?
                     buckets()[(hash_val + 1) % num_buckets] : dict_size);

    while (pos < end) {
      auto key = pos + sizeof(size_t);

      uint64_t key_len;
      key = (const char*) decode(key_len, (const unsigned char*) key);

      if (string_view(key, key_len) == word) {
        return pos;
      }

      pos = key + key_len;
    }

    return nullptr;
  }

  char *find_entry(const string_view &word) {
    return const_cast<char*>(static_cast<const dictionary*>(this)->find_entry(word));
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
