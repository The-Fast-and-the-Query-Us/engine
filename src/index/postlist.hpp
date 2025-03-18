#pragma once


#include "common.hpp"
#include "compress.hpp"
#include "list.hpp"

namespace fast {
/*
* in memory postlist serialization
*  should we store number of documents?
*
*  Format:
*  header, sync (uncompressed), posts (encoded)
*/
class postlist {
  static constexpr size_t MIN_SYNC = 100;

  size_t word_count, sync_len;

  size_t *sync() { return &sync_len + 1; }
  unsigned char   *posts() { return reinterpret_cast<unsigned char*>(sync() + sync_len); }

  public:

  static size_t size_needed(const list<uint64_t> &posts) {
    const auto syncs = fast_sqrt(posts.size());
    size_t dynamic{0};
    if (syncs >= MIN_SYNC) {
      dynamic += 2 * sizeof(size_t) * syncs;
    }

    size_t last = 0;
    for (const auto post : posts) {
      dynamic += encoded_size(post - last);
      last = post;
    }
    dynamic += encoded_size(0);

    return dynamic + sizeof(postlist);
  }

  static unsigned char *write(const list<uint64_t> &posts, postlist *buffer) {
    buffer->word_count = posts.size();
    const auto syncs = fast_sqrt(posts.size());

    if (syncs >= MIN_SYNC) {
      buffer->sync_len = syncs;
    } else {
      buffer->sync_len = 0;
    }

    auto write_pos = buffer->posts();
    size_t last = 0;
    size_t i    = 0;

    for (const auto post : posts) {
      if (syncs >= MIN_SYNC && i % syncs == 0) {
        buffer->sync()[i / syncs] = write_pos - buffer->posts();
      }
      write_pos = encode(post - last, write_pos);
      last = post;
      i++;
    }

    return write_pos;
  }
};

}
