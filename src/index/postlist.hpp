#pragma once


#include "common.hpp"
#include "compress.hpp"
#include "list.hpp"

namespace fast {
/*
* in memory postlist serialization
*
*  Format:
*  header, sync <post value, post offset>, posts <delta>\0
*/
class postlist {
  static constexpr size_t MIN_SYNC = 100;

  size_t word_count, post_len, sync_count;

  pair<size_t>  *sync()   { return reinterpret_cast<pair<size_t>*>(&sync_count + 1); }
  unsigned char *posts()  { return reinterpret_cast<unsigned char*>(sync() + sync_count); }

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
      buffer->sync_count = syncs;
    } else {
      buffer->sync_count = 0;
    }

    auto write_pos = buffer->posts();
    size_t last = 0;
    size_t i    = 0;

    for (const auto post : posts) {
      if (syncs >= MIN_SYNC && i % syncs == 0) {
        buffer->sync()[i / syncs] = {size_t(write_pos - buffer->posts()), post};
      }
      write_pos = encode(post - last, write_pos);
      last = post;
      i++;
    }

    write_pos = encode(0, write_pos);

    buffer->post_len = write_pos - buffer->posts();
    return write_pos;
  }

  class isr {
    friend class postlist;

    const unsigned char *buf;
    uint64_t acc;

    public:
    
    isr& operator++() {
      uint64_t adj;
      buf = decode(adj, buf);
      acc += adj;
      return *this;
    }

    uint64_t operator*() const {return acc;}

    bool operator==(const isr &other) const {
      return buf == other.buf;
    }
  };

  isr begin() {
    isr ans;
    ans.buf = decode(ans.acc, posts());
    return ans;
  }

  isr end() {
    isr ans;
    ans.buf = posts() + post_len;
    return ans;
  }

};

}
