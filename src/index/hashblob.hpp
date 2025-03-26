#pragma once

#include "common.hpp"
#include "dictionary.hpp"
#include "hashtable.hpp"
#include "postlist.hpp"
#include <cstddef>
namespace fast {

/*
* Format
* Dictionary, doc ends, <posting lists>
*/

class hashblob {
  size_t doc_offset;

public:
  static size_t size_required(const hashtable &ht) {
    size_t needed{sizeof(hashblob)};

    needed = round_up(needed, alignof(dictionary));
    needed += dictionary::size_required(ht);

    needed = round_up(needed, alignof(postlist<Doc>));
    needed += postlist<Doc>::size_needed(ht.docs);

    for (auto i = 0u; i < ht.num_buckets; ++i) {
      for (const auto &b : ht.buckets[i]) {
        needed = round_up(needed, alignof(postlist<Text>));
        needed += postlist<Text>::size_needed(b.posts);
      }
    }

    return needed;
  }

  static void write(const hashtable &ht, hashblob *buffer) {
    auto write_pos = dictionary::write(ht, buffer->dict());

    write_pos = align_ptr(write_pos, alignof(postlist<Doc>));
    buffer->doc_offset = write_pos - (char *)buffer;
    auto wp = postlist<Doc>::write(ht.docs, buffer->docs());

    for (auto i = 0ul; i < ht.num_buckets; ++i) {
      for (const auto &w : ht.buckets[i]) {
        wp = align_ptr(wp, alignof(postlist<Text>));
        buffer->dict()->put(w.word, wp - (unsigned char*)buffer);
        wp = postlist<Text>::write(w.posts, (postlist<Text>*) wp);
      }
    }

  }

  dictionary *dict() {
    return align_ptr(reinterpret_cast<dictionary*>(&doc_offset + 1), alignof(dictionary));
  }

  postlist<Doc> *docs() {
    return reinterpret_cast<postlist<Doc>*>(
      reinterpret_cast<char*>(this) + doc_offset
    );
  }

  postlist<Text> *get(const string_view &word) {
    const auto p = dict()->get(word);
    if (!p.second) {
      return nullptr;
    } else {
      return reinterpret_cast<postlist<Text>*>(
        (char*)this + p.first
      );
    }
  }
};

}


