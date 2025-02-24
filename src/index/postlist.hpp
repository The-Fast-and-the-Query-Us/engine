#pragma once

#include "compress.hpp"
#include "list.hpp"
#include <cstddef>
#include <cstring>
#include <hashtable.hpp>

namespace fast {
/*
* Compressed postlist without a sync table. MUST BE SIZE_T ALIGNED
*
* Format
* listSize (bytes), numWords, diff from last doc to num_docs
* <doc delta, <off delta> \0>
*/
class postlist {
  size_t listSize;
  size_t numWords;
  size_t delta;

  char *data() const { return reinterpret_cast<char*>(delta + 1); }

  public:

  static size_t size_needed(const list<hashtable::post> &posts) {
    size_t dynamic{0};
    size_t last_doc{0};

    auto it = posts.begin();

    for (auto base = posts.begin(); base != posts.end(); base = it) {
      const auto doc_delta = (*base).doc_id - last_doc; // add -> operator maybe
      dynamic += compressed_size(doc_delta) + 1; // +1 for null

      size_t last_offset{0};
      for (it = base; it != posts.end() && (*it).doc_id == (*base).doc_id; ++it) {
        dynamic += compressed_size((*it).offset - last_offset);
        last_offset = (*it).offset;
      }
      last_doc = (*base).doc_id;
    }

    return dynamic + sizeof(postlist);
  }

  // write posting list and return its end
  static char *write(const list<hashtable::post> &posts, postlist *buffer, size_t num_docs) {
    buffer->numWords = posts.length();

    size_t last_doc{0};
    auto it = posts.begin();
    auto writePos = buffer->data();

    size_t biggest_doc{0};
    for (auto base = posts.begin(); base != posts.end(); base = it) {
      writePos = compress((*base).doc_id - last_doc, writePos);

      size_t last_offset{0};
      for (it = base; it != posts.end() && (*it).doc_id == (*base).doc_id; ++it) {
        writePos = compress((*it).offset - last_offset, writePos);
      }

      *(writePos++) = 0;
      if ((*base).doc_id > biggest_doc) biggest_doc = (*base).doc_id;
    }

    buffer->listSize = writePos - buffer->data();
    buffer->delta = num_docs - biggest_doc;

    return writePos;
  }

  // size needed to merge
  static size_t size_needed(const postlist *l, const postlist *r) {
    size_t rstart;
    expand(rstart, r->data());

    return sizeof(postlist) + l->listSize + r->listSize + compressed_size(l->delta + rstart) - compressed_size(rstart);
  }

  // merge two post lists together
  static char *merge(const postlist *l, const postlist *r, postlist *buffer) {
    buffer->delta = r->delta;
    buffer->numWords = l->numWords + r->numWords;

    auto writePos = buffer->data();

    memcpy(writePos, l->data(), l->listSize);
    writePos += l->listSize;

    size_t rstart; 
    auto rPos = expand(rstart, r->data());

    writePos = compress(rstart + l->delta, writePos);

    size_t amount = r->listSize - (rPos - r->data());
    memcpy(writePos, rPos, amount);
    writePos += amount;

    buffer->listSize = writePos - buffer->data();
    return writePos;
  }

  struct post {
    size_t doc;
    size_t offset;
  };

  class isr {
    size_t cur_doc, cur_offset;
    char *pos, *end;
    public:
    isr& operator++() {
      pos = skip_compressed(pos);

      if (*pos == 0) {
        if (++pos == end) return *this;

        size_t doc_delta;
        pos = expand(doc_delta, pos);
        cur_doc += doc_delta;
      }

      size_t offset_delta;
      expand(offset_delta, pos);
      cur_offset += offset_delta;

      return *this;
    }

    post operator*() const {
      return {cur_doc, cur_offset};
    }
  };
};
}
