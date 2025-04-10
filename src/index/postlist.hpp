#pragma once


#include "common.hpp"
#include "compress.hpp"
#include "list.hpp"

namespace fast {
/*
* in memory postlist serialization
*
*  Format:
*  header, sync (uncompressed) <post value, post offset>, posts <delta>\0
*/
class postlist {
  uint64_t word_count, post_len, sync_count;

  pair<uint64_t>  *sync()   { return reinterpret_cast<pair<uint64_t>*>(&sync_count + 1); }

  // sync_count must be init first
  unsigned char *posts()  { return reinterpret_cast<unsigned char*>(sync() + sync_count); }

  public:

  uint64_t words() const { return word_count; }

  static size_t size_needed(const list<uint64_t> &posts) {
    size_t dynamic{0};

    const auto sync_count = fast_sqrt(posts.size());
    dynamic += sizeof(pair<uint64_t>) * sync_count;

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
    buffer->sync_count = fast_sqrt(posts.size());

    auto write_pos = buffer->posts();
    uint64_t last = 0;
    size_t i      = 0;

    const auto POST_PER_SYNC = posts.size() / buffer->sync_count;

    for (const auto post : posts) {
      write_pos = encode(post - last, write_pos);

      if ((i + 1) % POST_PER_SYNC == 0) {
        buffer->sync()[i / POST_PER_SYNC] = {uint64_t(write_pos - buffer->posts()), post};
      }

      last = post;
      i++;
    }

    write_pos = encode(0, write_pos);

    buffer->post_len = write_pos - buffer->posts();
    return write_pos;
  }

  // index stream reader
  // Todo: figure out URL retrieval
  class isr {
    friend class postlist;

    const unsigned char *buf; // points to start of next delta to be read
    uint64_t acc; // acc(umulator) to total all the deltas

    isr() {}
    isr(const unsigned char *buf, uint64_t acc) : buf(buf), acc(acc) {}

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

  isr upper_bound(uint64_t search) {
    size_t post_offset = 0;
    uint64_t acc = 0;

    auto table = sync();
    for (size_t i = 0; i < sync_count; ++i) {
      if (table[i].second > search) break;
      post_offset = table[i].first;
      acc = table[i].second;
    }

    isr ans{posts() + post_offset, acc};

    while (ans != end() && *ans <= search) ++ans;

    return ans;
  }

};

}
