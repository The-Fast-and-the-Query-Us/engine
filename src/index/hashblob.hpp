#pragma once

#include <cstddef>
#include <hashtable.hpp>

namespace fast {

class hashblob {
  size_t magic, num_buckets, data;
  public:

  struct dict_entry {
    size_t len;
    size_t posts;
    uint32_t hash_val;
    char word;

    static size_t serial_size(hashtable::bucket *b) {
      return round_up(sizeof(dict_entry) + b->word.length(), sizeof(size_t));
    }
  };
};

}


