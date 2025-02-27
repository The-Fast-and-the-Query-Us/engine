#pragma once

#include "string.hpp"
#include <cstddef>
#include <cstdint>

#include <list.hpp>
#include <hash.hpp>

namespace fast {

class hashtable {
  struct bucket {
    uint64_t hashval;
    string word;
    list<uint64_t> posts;

    bucket(uint64_t hashval, const string &word) : hashval(hashval), word(word) {}
  };

  size_t num_buckets, next_offset;
  list<bucket> *buckets;

  friend class dictionary;

public:
  hashtable(size_t num_buckets = 2048) : num_buckets(num_buckets), next_offset(0) {
    buckets = new list<bucket>[num_buckets];
  }

  hashtable(const hashtable &ht) = delete;

  hashtable &operator=(const hashtable &ht) = delete;

  ~hashtable() {
    delete [] buckets;
  }

  size_t tokens() const { return next_offset; }

  void add(const string &word) {
    const auto hashval = hash(word.c_str());

    list<bucket> &l = buckets[hashval % num_buckets];

    for (auto &b : l) {
      if (b.hashval == hashval && b.word == word) {
        b.posts.push_back(next_offset++);
        return;
      }
    }
    l.emplace_back(hashval, word);
    l.back().posts.push_back(next_offset++);
  }
};

}
