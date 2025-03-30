#pragma once

#include "compress.hpp"
#include "list.hpp"
#include "hashtable.hpp"
#include <pair.hpp>

namespace fast {

class post_list {
  size_t num_words, sync_len, len;

  pair<size_t> *sync() { return reinterpret_cast<pair<size_t>*>(&len + 1); }

  const pair<size_t> *sync() const { return reinterpret_cast<const pair< size_t>*>(&len + 1); }

  unsigned char *posts() { return reinterpret_cast<unsigned char*>(sync() + sync_len); }

  const unsigned char *posts() const {
    return reinterpret_cast<const unsigned char*>(sync() + sync_len);
  }

  // open questions :
  // should we use sqrt?
  // should we binary search on this (because we can but we only seek forward)
  // should we go by offset or number of posts (by offset we can jump directly into the table)
  static size_t get_per_sync(const list<Offset> &posts) {
    (void) posts; // for compile
    return 5'000;
  }

public:

  size_t words() const { return num_words; }

  static size_t size_needed(const list<Offset> &posts) {
    const auto PER_SYNC = get_per_sync(posts);

    size_t dynamic = 0;
    dynamic += posts.size() / PER_SYNC * sizeof(pair<size_t>);

    Offset last = 0;
    for (const auto post : posts) {
      dynamic += encoded_size(post - last);
      last = post;
    }

    dynamic += encoded_size(0);

    return dynamic + sizeof(post_list);
  }

  static unsigned char *write(const list<Offset> &posts, post_list *buffer) {
    const auto PER_SYNC = get_per_sync(posts);

    buffer->num_words = posts.size();
    buffer->sync_len = posts.size() / PER_SYNC;

    auto wp = buffer->posts();

    size_t idx = 0;
    Offset last = 0;

    for (const auto post : posts) {
      wp = encode(post - last, wp);
      ++idx;
      last = post;

      if (idx % PER_SYNC == 0) {
        buffer->sync()[idx / PER_SYNC - 1] = {size_t(wp - buffer->posts()), post};
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

    size_t next_sync;
    const post_list *pl;

    isr(Offset base, const unsigned char *buff, size_t next_sync, const post_list *pl) 
    : acc(base), buff(buff), next_sync(next_sync), pl(pl) {}

    isr(const unsigned char *buff) : buff(buff) {};

  public:

    Offset operator*() const { return acc; }

    operator bool() const { return *this != pl->end(); }

    isr& operator++() {
      uint64_t tmp;
      buff = decode(tmp, buff);
      acc += tmp;
      return *this;
    }

    // seek forward to first post >= offset
    void seek_forward(Offset offset) {
      for (; next_sync < pl->sync_len; ++next_sync) {
        const auto sync = pl->sync()[next_sync];
        if (sync.second > offset) break;

        buff = pl->posts() + sync.first;
        acc = sync.second;
      }

      while (*this && **this < offset) ++(*this);
    }

    bool operator==(const isr &other) const {
      return buff == other.buff;
    }
  };

  isr begin() const {
    isr ans(0, posts(), 0, this);
    return ++ans;
  }

  isr end() const {
    return isr(posts() + len);
  }

};

}
