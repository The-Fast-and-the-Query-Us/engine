#pragma once

#include "hash.hpp"
#include "list.hpp"
#include <cstddef>
#include <cstdint>

namespace fast {

// TODO add rehashing
template <class K, class V>
class hashmap {

  struct node {
    uint64_t hashval;
    K key;
    V val;

    node(uint64_t hashval, K key) : hashval(hashval), key(key), val() {}
  };

  list<node> *buckets;
  size_t num_buckets;
  size_t num_nodes;

  public:

  hashmap(size_t init_buckets = 32) {
    num_buckets = init_buckets;
    buckets = new list<node>[init_buckets];
    num_nodes = 0;
  }

  ~hashmap() { delete []buckets; }

  // number of key, val pairs in the map
  size_t size() const { return num_nodes; }

  V& operator[](const K &key) {
    const auto hashval = hash(key);
    const auto bucket = hashval % num_buckets;

    for (auto &node : buckets[bucket]) {
      if (node.hashval == hashval && node.key == key) return node.val;
    }

    ++num_nodes;
    buckets[bucket].emplace_back(hashval, key);
    return buckets[bucket].back().val;
  }
};

}
