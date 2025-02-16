#pragma once

#include <cstddef>
#include <cstdint>
#include <list.hpp>
#include "static_string.hpp"

namespace fast {

class hashtable {

  public:
  struct post {
    uint32_t doc_id;
    uint32_t offset;
  };

  struct bucket {
    uint32_t hash_val;
    static_string word;
    list<post> posts;
  };

  private:

  size_t num_buckets_;
  list<bucket> *buckets_;

  public:
  hashtable(size_t num_buckets = 2048) : num_buckets_(num_buckets) {
    buckets_ = new list<bucket>[num_buckets];
  };

  hashtable(const hashtable &other) = delete;

  hashtable& operator=(const hashtable &other) = delete;

  ~hashtable() {
    delete[] buckets_;
  }

  static uint32_t hash(const static_string &s) {
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

  class iterator {
    list<bucket>::iterator it;
    list<bucket>::iterator end;
    hashtable *ht;
    size_t cur_list;
    public:

    const bucket& operator*() {
      return *it;
    }

    iterator& operator++() {
      if (++it != end) {
        return *this;
      }

      for (++cur_list; cur_list < ht->num_buckets_; ++cur_list) {
        if (ht->buckets_[cur_list].length() > 0) {
          it = ht->buckets_[cur_list].begin();
          end = ht->buckets_[cur_list].end();
          break;
        }
      }

      return *this;
    }

    bool operator!=(const iterator &other) {
      (void) other;
      return false; // todo
    }
  };

};
}
