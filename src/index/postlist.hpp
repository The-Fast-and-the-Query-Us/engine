#pragma once

#include "common.hpp"
#include "list.hpp"
#include <cstddef>
#include <hashtable.hpp>

namespace fast {
/*
* Format
*
*/
class postlist {
  public:

  static size_t size_needed(const list<hashtable::post> &posts) {
    for (const auto &post : posts) {
      
    }

    return 2 * sizeof(size_t) + 0 + 0; // TODO
  }

  // write posting list and return its end
  static char *write(const list<hashtable::post> &posts, char *buffer) {
    (void)posts;
    return buffer;
  }
};
}
