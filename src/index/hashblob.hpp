#pragma once

#include <cstddef>
#include <hashtable.hpp>

namespace fast {

/*
* Hashblob structure
* HEADERS
* buckets [ ] -> dict entrys
* dictentrys[ ] = compressed size_t (posting start) + word + \0
*/
class hashblob {
  size_t magic, num_buckets, dict_end;
  char data;

  public:

  static void write(hashtable *ht, hashblob *buffer, size_t num_buckets) {
  }
};

}


