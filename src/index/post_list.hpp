#pragma once

#include "compress.hpp"
#include "list.hpp"
#include "hashtable.hpp"

namespace fast {

class post_list {
  static constexpr size_t PER_SYNC = 5'000;

  size_t num_words, sync_len, len;

  pair<size_t> *sync() { return reinterpret_cast<pair<size_t>*>(&len + 1); }

  const pair<size_t> *sync() const { return reinterpret_cast<const pair< size_t>*>(&len + 1); }

  unsigned char *posts() { return reinterpret_cast<unsigned char*>(sync() + sync_len); }

  const unsigned char *posts() const {
    return reinterpret_cast<const unsigned char*>(sync() + sync_len);
  }

public:

  static size_t size_needed(const list<Offset> &posts) {
    size_t dynamic = 0;

    dynamic += posts.size() / PER_SYNC * sizeof(pair<size_t>);

    Offset last = 0;
    for (const auto post : posts) {
      dynamic += encoded_size(post - last);
      last = post;
    }

    return dynamic + sizeof(post_list);
  }

  static unsigned char *write(const list<Offset> &posts, post_list *buffer) {
    buffer->num_words = posts.size();

    const int SYNC_LEN = posts.size() / PER_SYNC;
    const int POSTS_PER = posts.size() / SYNC_LEN;

    buffer->sync_len = SYNC_LEN;

    auto wp = buffer->posts();

    size_t idx = 0;
    Offset last = 0;

    for (const auto post : posts) {
      wp = encode(post - last, wp);
      ++idx;
      last = post;

      if (idx % POSTS_PER == 0) {
        buffer->sync()[idx / POSTS_PER - 1] = {size_t(wp - buffer->posts()), post};
      }
    }

    wp = encode(0, wp);
    buffer->len = size_t(wp - buffer->posts());
    return wp;
  }

  class isr {
    friend class post_list;

    Offset acc;
    const unsigned char *buff;

    isr(Offset base, const unsigned char *buff) : acc(base), buff(buff) {}

  public:

    Offset operator*() const { return acc; }

    isr& operator++() {
      uint64_t tmp;
      buff = decode(tmp, buff);
      acc += tmp;
      return *this;
    }
  };

  isr begin() const {
    isr ans(0, posts());
    return ++ans;
  }

  isr end() const {
    return isr(0, posts() + len);
  }

};

}
