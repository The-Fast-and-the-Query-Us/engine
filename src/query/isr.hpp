#pragma once

#include <hashtable.hpp>
#include <post_list.hpp>
#include <isr.hpp>

namespace fast::query {

/*
* Isr should implement
* void next()
* void seek(Offset)
* Offset offset()
* bool is_end()
*/

// add phrase, OR, and AND
class isr_or : public isr {

  isr **streams;
  size_t count;

  public:

  isr_or(isr **streams, size_t count) : streams(streams), count(count) {}

  void next() override {
    Offset small = MAX_OFFSET;
    size_t small_idx;

    for (auto i = 0u; i < count; ++i) {
      if (streams[i]->is_end()) continue;

      const auto this_offset = streams[i]->offset();
      if (this_offset < small) {
        small = this_offset;
        small_idx = i;
      }
    }

    streams[small_idx]->next();
  }

  void seek(Offset offset) override {
    for (auto i = 0u; i < count; ++i) {
      streams[i]->seek(offset);
    }
  }

  Offset offset() override {
    auto ans = MAX_OFFSET;
    for (auto i = 0u; i < count; ++i) {
      if (!is_end()) ans = min(ans, streams[i]->offset());
    }
    return ans;
  }
  
  bool is_end() override {
    for (auto i = 0u; i < count; ++i) {
      if (!streams[i]->is_end()) return false;
    }
    return true;
  }
};

class isr_and : public isr {
  public:
};

class isr_phrase : public isr {

  isr *stream;
  size_t count;

  public:

};

}
