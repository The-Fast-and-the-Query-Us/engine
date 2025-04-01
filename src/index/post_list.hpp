#pragma once

#include "compress.hpp"
#include "list.hpp"
#include "hashtable.hpp"
#include <cstring>
#include <pair.hpp>
#include <type_traits>
#include "isr.hpp"

namespace fast {

namespace hidden {

template<class T>
inline size_t post_size(const T &elt, Offset last);

template<>
inline size_t post_size(const Offset &elt, Offset last) {
  return encoded_size(elt - last);
}

template<>
inline size_t post_size(const url_post &elt, Offset last) {
  size_t dyn = 0;
  dyn += encoded_size(elt - last);
  dyn += encoded_size(elt.doc_len);
  dyn += encoded_size(elt.url.size());
  dyn += elt.url.size();
  return dyn;
}

template<class T>
inline unsigned char *write_post(const T &elt, Offset last, unsigned char *wp);

template<>
inline unsigned char *write_post(const Offset &elt, Offset last, unsigned char *wp) {
  return encode(elt - last, wp);
}

template<>
inline unsigned char *write_post(const url_post &elt, Offset last, unsigned char *wp) {
  wp = encode(elt.offset - last, wp);
  wp = encode(elt.doc_len, wp);
  wp = encode(elt.url.size(), wp);
  memcpy(wp, elt.url.begin(), elt.url.size());
  return wp + elt.url.size();
}

}

class post_list {
  bool is_doc;
  Offset last;
  size_t num_words, sync_len, len;

  pair<size_t> *sync() { return reinterpret_cast<pair<size_t>*>(&len + 1); }

  const pair<size_t> *sync() const { return reinterpret_cast<const pair< size_t>*>(&len + 1); }

  unsigned char *posts() { return reinterpret_cast<unsigned char*>(sync() + sync_len); }

  const unsigned char *posts() const {
    return reinterpret_cast<const unsigned char*>(sync() + sync_len);
  }

  template<class T>
  static size_t get_per_sync(const list<T> &posts) {
    (void) posts; // for compile
    return 5'000;
  }

public:
  Offset get_last() const { return last; }

  size_t words() const { return num_words; }

  template<class T>
  static size_t size_needed(const list<T> &posts) {
    const auto PER_SYNC = get_per_sync(posts);

    size_t dynamic = 0;
    dynamic += posts.size() / PER_SYNC * sizeof(pair<size_t>);

    Offset last = 0;
    for (const auto &post : posts) {
      dynamic += hidden::post_size(post, last);
      last = post;
    }
    return dynamic + sizeof(post_list);
  }

  template<class T>
  static unsigned char *write(const list<T> &posts, post_list *buffer) {
    const auto PER_SYNC = get_per_sync(posts);

    buffer->num_words = posts.size();
    buffer->sync_len = posts.size() / PER_SYNC;

    auto wp = buffer->posts();

    size_t idx = 0;
    Offset last = 0;

    for (const auto &post : posts) {
      wp = hidden::write_post(post, last, wp);
      ++idx;
      last = post;

      if (idx % PER_SYNC == 0) {
        buffer->sync()[idx / PER_SYNC - 1] = {size_t(wp - buffer->posts()), post};
      }
    }
    
    wp = encode(0, wp); // zero terminator

    buffer->len = size_t(wp - buffer->posts());
    buffer->last = posts.back();

    if constexpr (std::is_same_v<T, url_post>) {
      buffer->is_doc = true;
    } else {
      buffer->is_doc = false;
    }

    return wp;
  }

  isr *get_isr() const {
    if (is_doc) {
      return new isr_doc(len, posts(), sync(), sync() + sync_len);
    } else {
      return new isr_word(len, posts(), sync(), sync() + sync_len);
    }
  }

  isr_doc *get_doc_isr() const {
    return new isr_doc(len, posts(), sync(), sync() + sync_len);
  }

};

}
