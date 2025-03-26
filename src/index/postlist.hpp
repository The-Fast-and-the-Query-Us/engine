#pragma once


#include "common.hpp"
#include "list.hpp"
#include "types.hpp"

namespace fast {

class postlist {
  uint64_t word_count, post_len, sync_count;

  pair<uint64_t> *sync()   { 
    return reinterpret_cast<pair<uint64_t>*>(&sync_count + 1); 
  }

  // sync_count must be init first
  unsigned char *posts()  { 
    return reinterpret_cast<unsigned char*>(sync() + sync_count);
  }

  public:

  uint64_t words() const { return word_count; }

  template <post_type PT>
  static size_t size_needed(const list<post<PT>> &posts) {
    size_t dynamic{0};

    const auto sync_count = fast_sqrt(posts.size());
    dynamic += sizeof(pair<uint64_t>) * sync_count;

    uint64_t last = 0;
    for (const auto &post : posts) {
      dynamic += post.size_required(last);
      last = post;
    }

    dynamic += post<PT>::term_size();

    return dynamic + sizeof(postlist);
  }

  template <post_type PT>
  static unsigned char *write(const list<post<PT>> &posts, postlist *buffer) {
    buffer->word_count = posts.size();
    buffer->sync_count = fast_sqrt(posts.size());

    auto write_pos = buffer->posts();
    uint64_t last = 0;
    size_t i      = 0;

    const auto POST_PER_SYNC = posts.size() / buffer->sync_count;

    for (const auto &post : posts) {
      write_pos = post.write(write_pos, last);

      if ((i + 1) % POST_PER_SYNC == 0) {
        buffer->sync()[i / POST_PER_SYNC] = {
          uint64_t(write_pos - buffer->posts()), 
          post
        };
      }

      last = post;
      i++;
    }

    write_pos = post<PT>::write_term(write_pos);

    buffer->post_len = write_pos - buffer->posts();
    return write_pos;
  }


  template<post_type PT>
  isr<PT> begin() {
    return isr<PT>(posts(), 0);
  }

  template<post_type PT>
  isr<PT> end() {
    return isr<PT>(posts() + post_len);
  }

};

}
