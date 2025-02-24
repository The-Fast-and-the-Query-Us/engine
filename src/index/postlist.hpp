#pragma once

#include "compress.hpp"
#include "list.hpp"
#include <cstddef>
#include <hashtable.hpp>

namespace fast {
/*
* Compressed postlist without a sync table
*
* Format
* <doc delta, <off delta> \0>\0 
*/
class postlist {
  public:

  static size_t size_needed(const list<hashtable::post> &posts) {
    size_t dynamic{0};
    size_t last_doc{0};

    auto it = posts.begin();

    for (auto base = posts.begin(); base != posts.end(); base = it) {
      const auto doc_delta = (*base).doc_id - last_doc; // add -> operator maybe
      dynamic += compressed_size(doc_delta) + 1; // +1 for null

      size_t last_offset{0};
      for (it = base; it != posts.end() && (*it).doc_id == (*base).doc_id; ++it) {
        dynamic += compressed_size((*it).offset - last_offset);
        last_offset = (*it).offset;
      }
      last_doc = (*base).doc_id;
    }

    return dynamic + 1; // +1 for null
  }

  // write posting list and return its end
  static char *write(const list<hashtable::post> &posts, char *buffer) {
    (void)posts;
    return buffer;
  }
};
}
