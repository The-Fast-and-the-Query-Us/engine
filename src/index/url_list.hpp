#pragma once

#include "common.hpp"
#include "list.hpp"
namespace fast {

class url_list {
  size_t num_urls, num_buckets, len;

public:
  static size_t size_needed(const list<pair<uint64_t, string>> &urls);

  static unsigned char *write(const list<pair<uint64_t, string>> &urls);
};

}
