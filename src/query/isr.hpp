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
      if (!streams[i]->is_end()) ans = min(ans, streams[i]->offset());
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

// use this for AND and NOT
class isr_container : public isr {
  isr **include, **exclude;
  unsigned count_inc, count_ex;
  isr_doc *doc_end;

  public:

  void next() override {
    seek(doc_end->offset()); // should we check here?
  }

  void seek(Offset offset) override {
        
    // 1. Seek all the included ISRs to the first occurrence beginning at
    // the target location.
    for (auto i = 0u; i < count_inc; ++i) {
      include[i]->seek(offset);
      if (include[i]->is_end()) return;
    }

    while (1) {

      // 2. Move the document end ISR to just past the furthest
      // contained ISR, then calculate the document begin location.
      Offset farthest = 0;
      for (auto i = 0u; i < count_inc; ++i) {
        farthest = max(farthest, include[i]->offset());
      }

      doc_end->seek(farthest + 1);
      if (doc_end->is_end()) return;
      const auto doc_begin = doc_end->offset() - doc_end->len();


      // 3. Seek all the other contained terms to past the document begin.
      bool good = true;
      for (auto i = 0u; i < count_inc && good; ++i) {
        include[i]->seek(doc_begin);

        // 5. If any ISR reaches the end, there is no match.
        if (include[i]->is_end()) return;

        // 4. If any contained erm is past the document end, return to
        // step 2.
        if (include[i]->offset() > doc_end->offset()) good = false;
      }
    }

    const auto doc_begin = doc_end->offset() - doc_end->len();
    //
    // 6. Seek all the excluded ISRs to the first occurrence beginning at
    // the document begin location.
    for (auto i = 0u; i < count_ex; ++i) {
      if (exclude[i]->is_end()) continue;

      exclude[i]->seek(doc_begin);

      // 7. If any excluded ISR falls within the document, reset the
      // target to one past the end of the document and return to
      // step 1.
      if (exclude[i]->offset() < doc_end->offset()) {
        return seek(doc_end->offset());

      }
    }
  }

  Offset offset() override {
    return include[0]->offset();
  }

  bool is_end() override {
    for (auto i = 0u; i < count_inc; ++i) {
      if (include[i]->is_end()) return true;
    }
    return false;
  }
};

class isr_phrase : public isr {

  isr **streams;
  size_t count;

  public:

  isr_phrase(isr **streams, size_t count) : streams(streams), count(count) {
    next();
  }

  void next() override {
    seek(offset() + 1);
  }

  void seek(Offset offset) override {
    for (auto i = 0u; i < count; ++i) {
      streams[i]->seek(offset);
    }

    while (!streams[count - 1]->is_end()) {
      const auto base = streams[count - 1]->offset();
      bool good = true;
      for (auto i = 0u; i < count - 1 && good; ++i) {
        const auto goal = base - count + i + 1;
        streams[i]->seek(goal);

        if (streams[i]->is_end()) return;
        else if (streams[i]->offset() != goal) {
          streams[count - 1]->next();
          good = false;
        }
      }

      if (good) return;
    }
  }

  Offset offset() override {
    return streams[0]->offset();
  }

  bool is_end() override {
    for (auto i = 0u; i < count; ++i) {
      if (streams[i]->is_end()) return true;
    }
    return false;
  }
};

}
