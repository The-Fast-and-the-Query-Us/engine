#pragma once

#include "list.hpp"

namespace fast {

class post_list {
  size_t num_words, sync_len, len;

  size_t *sync() { return &len + 1; }
  const size_t *sync() const { return &len + 1; }

public:

  static size_t size_needed(const list<uint32_t> &posts);

  static unsigned char *write(const list<uint32_t> &posts, post_list *buffer);
};

}
