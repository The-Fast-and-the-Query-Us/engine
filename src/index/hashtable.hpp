#pragma once

#include <cstddef>
#include <cstdint>
#include <list.hpp>
#include "hashblob.hpp"
#include "static_string.hpp"

namespace fast {
struct post {
  uint32_t doc_id;
  uint32_t offset;
};


class hashtable {

  struct bucket {
    uint32_t hash_val;
    static_string word;
    list<post> posts;
  };

  size_t num_buckets_;
  list<bucket> *buckets_;

  friend class hashblob;

  public:
  hashtable(size_t num_buckets = 2048) : num_buckets_(num_buckets) {
    buckets_ = new list<bucket>[num_buckets];
  };

  hashtable(const hashtable &other) = delete;

  hashtable& operator=(const hashtable &other) = delete;

  ~hashtable() {
    delete[] buckets_;
  }

  static uint32_t hash(static_string &s) {
    const uint32_t P = 101;

    uint32_t hash_val = 0;
    uint32_t ppow = 1;

    for (auto c : s) {
      hash_val += ppow * c;
      ppow *= P;
    }

    return hash_val;
  }

  void add(static_string &word, post p) {
    const auto hash_val = hash(word);
    list<bucket> &l = buckets_[hash_val % num_buckets_];

    for (auto &bucket : l) {
      if (bucket.hash_val == hash_val && bucket.word == word) {
        bucket.posts.push_back(p);
        return;
      }
    }

    l.push_back({hash_val, word, list<post>()});
    l.back()->posts.push_back(p);
  }

  const bucket* get(static_string &word) {
    const auto hash_val = hash(word);
    list<bucket> &l = buckets_[hash_val % num_buckets_];

    for (auto &bucket : l) {
      if (bucket.hash_val == hash_val && bucket.word == word) {
        return &bucket;
      }
    }

    return nullptr;
  }
};
}
