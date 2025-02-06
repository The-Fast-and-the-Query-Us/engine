#pragma once

#include <cstddef>
#include <cstdint>
#include <list.hpp>
#include "static_string.hpp"

namespace fast {
struct post {
  uint32_t doc_id;
  uint32_t offset;
};


class hashtable {

  struct bucket {
    static_string word;
    list<post> posts;
  };

  size_t num_buckets_;
  list<bucket> *buckets_;

  public:
  hashtable(size_t num_buckets) : num_buckets_(num_buckets) {

  };

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

  }
};
}
