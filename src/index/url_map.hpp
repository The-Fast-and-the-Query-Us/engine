#pragma once

#include "common.hpp"
#include "compress.hpp"
#include "list.hpp"
#include "string.hpp"
#include "hashtable.hpp"
#include "string_view.hpp"

namespace fast {

class url_map {
  size_t num_urls, len;

  size_t *buckets() {
    return &len + 1;
  }

  const size_t *buckets() const {
    return &len + 1;
  }

  unsigned char *data() {
    return reinterpret_cast<unsigned char *>(buckets() + num_urls);
  }

  const unsigned char *data() const {
    return reinterpret_cast<const unsigned char *>(buckets() + num_urls);
  }

public:
  static size_t size_needed(const list<Url> &urls) {
    size_t dynamic = 0;

    dynamic += urls.size() * sizeof(size_t);

    for (const auto &url : urls) {
      dynamic +=
        sizeof(url.second) + url.first.size() + encoded_size(url.first.size());
    }

    return dynamic + sizeof(url_map);
  }

  static unsigned char *write(const list<Url> &urls, url_map *buffer) {
    
  }

  // number of urls in this list
  size_t count() const { return num_urls; }

  const string_view get(uint64_t offset) const {

  }
};

}
