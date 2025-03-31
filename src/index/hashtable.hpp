#pragma once

#include "string.hpp"
#include <cstddef>
#include <cstdint>
#include <list.hpp>
#include <hash.hpp>
#include <pair.hpp>

namespace fast {

typedef uint32_t Offset;

struct url_post {
  Offset doc_len;
  Offset offset;
  string url;

  operator Offset() const { return offset; }
};

class hashtable {

  struct bucket {
    uint64_t hashval;
    string word;
    list<Offset> posts;

    bucket(uint64_t hashval, const string_view &word) 
    : hashval(hashval), word(word) {}
  };

  size_t num_buckets, next_offset, unique_words, doc_size;
  list<bucket> *buckets;

  list<url_post> docs;

  friend class dictionary;
  friend class hashblob;
public:
  hashtable(size_t num_buckets = 2048) : num_buckets(num_buckets), next_offset(0), unique_words(0), doc_size(0) {
    buckets = new list<bucket>[num_buckets];
  }

  hashtable(const hashtable &ht) = delete;

  hashtable &operator=(const hashtable &ht) = delete;

  ~hashtable() { delete[] buckets; }

  size_t tokens() const { return next_offset; }

  size_t unique() const { return unique_words; }

  void add(const string_view &word) {
    ++doc_size;
    const auto hashval = hash(word);

    list<bucket> &l = buckets[hashval % num_buckets];

    for (auto &b : l) {
      if (b.hashval == hashval && b.word == word) {
        b.posts.push_back(next_offset++);
        return;
      }
    }
    l.emplace_back(hashval, word);
    l.back().posts.push_back(next_offset++);
    ++unique_words;
  }

  void add_doc(const string_view &url) {
    docs.emplace_back(doc_size, next_offset++, url);
    doc_size = 0;
  }
};

} // namespace fast
