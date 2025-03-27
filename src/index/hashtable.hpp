#pragma once

#include "string.hpp"
#include <cstddef>
#include <cstdint>
#include <list.hpp>
#include <hash.hpp>

namespace fast {

typedef uint32_t Offset;
typedef pair<string, Offset> Url;

class hashtable {

  struct bucket {
    uint64_t hashval;
    string word;
    list<Offset> posts;

    bucket(uint64_t hashval, const string_view &word) 
    : hashval(hashval), word(word) {}
  };

  size_t num_buckets, next_offset, unique_words;
  list<bucket> *buckets;

  friend class dictionary;

public:


  hashtable(size_t num_buckets = 2048) : num_buckets(num_buckets), next_offset(0), unique_words(0) {
    buckets = new list<bucket>[num_buckets];
  }

  hashtable(const hashtable &ht) = delete;

  hashtable &operator=(const hashtable &ht) = delete;

  ~hashtable() {
    delete [] buckets;
  }

  size_t tokens() const { return next_offset; }

  size_t unique() const { return unique_words; }

  void add(const string_view &word) {
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
};

}
