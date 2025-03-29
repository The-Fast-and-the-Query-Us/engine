#pragma once

#include "common.hpp"
#include "hashtable.hpp"
#include <cstddef>
#include "dictionary.hpp"
#include "post_list.hpp"

namespace fast {

class hashblob {
  static constexpr size_t MAGIC = 42;

  size_t magic;

public:

  const dictionary *dict() const {
    return reinterpret_cast<const dictionary*>(align_ptr(&magic + 1, alignof(dictionary)));
  }

  dictionary *dict() {
    return const_cast<dictionary*>(
      static_cast<const hashblob*>(this)->dict()
    );
  }

  bool is_good() const { return magic == MAGIC; }

  static size_t size_needed(const hashtable &ht) {
    size_t needed = sizeof(hashblob);

    needed = round_up(needed, alignof(dictionary));
    needed += dictionary::size_needed(ht);

    for (size_t i = 0; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        needed = round_up(needed, alignof(post_list));
        needed += post_list::size_needed(bucket.posts);
      }
    }

    return needed;
  }

  // buffer must be zero init and alignof(size_t) (maybe just init for caller?)
  static unsigned char *write(const hashtable &ht, hashblob *buffer) {
    auto write_pos = dictionary::write(ht, buffer->dict());

    for (size_t i = 0; i < ht.num_buckets; ++i) {
      for (const auto &bucket : ht.buckets[i]) {
        write_pos = align_ptr(write_pos, alignof(post_list));
        buffer->dict()->put(bucket.word, size_t(write_pos - reinterpret_cast<unsigned char*>(buffer)));
        write_pos = post_list::write(bucket.posts, reinterpret_cast<post_list*>(write_pos));
      }
    }

    buffer->magic = MAGIC;
    return write_pos;
  }

  const post_list *get(const string_view word) const {
    const auto p = dict()->get(word);
    if (p.second) {
      return reinterpret_cast<const post_list *>(
        reinterpret_cast<const char*>(this) + p.first
      );
    } else {
      return nullptr;
    }
  }
};

}
