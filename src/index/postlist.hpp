#pragma once


#include "list.hpp"
namespace fast {
/*
* in memory postlist serialization
*  should we store number of documents?
*/
class postlist {
  size_t word_count;
  public:

  static size_t size_needed(const list<uint64_t> &posts);

  static char *write(const list<uint64_t> &posts, postlist *buffer);
};

}
