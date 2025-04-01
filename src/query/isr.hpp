#pragma once

#include <hashtable.hpp>
#include <post_list.hpp>
#include <isr.hpp>
#include <vector.hpp>

namespace fast::query {

/*
* Isr should implement
* void next()
* void seek(Offset)
* Offset offset()
* bool is_end()
*/

class isr_or : public isr {

  vector<isr*> streams;
  public:

  void add_stream(isr *stream) {
    streams.push_back(stream);
  }

  void next() override {
    Offset small = MAX_OFFSET;
    size_t small_idx;

    for (auto i = 0u; i < streams.size(); ++i) {
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
    for (auto s : streams) {
      s->seek(offset);
    }
  }

  Offset offset() override {
    auto ans = MAX_OFFSET;
    for (auto s : streams) {
      if (!s->is_end()) ans = min(ans, s->offset());
    }
    return ans;
  }
  
  bool is_end() override {
    for (auto s : streams) {
      if (!s->is_end()) return false;
    }
    return true;
  }

  ~isr_or() override {
    for (const auto stream : streams) delete stream;
  }
};

// use this for AND and NOT (seek 0 to init)
class isr_container : public isr {
  vector<isr*> include, exclude;
  isr_doc *doc_end;

  public:

  void add_stream(isr *stream, bool ex = false) {
    if (ex) {
      exclude.push_back(stream);
    } else {
      include.push_back(stream);
    }
  }

  // jumps to next document
  void next() override { // maybe wrong
    seek(doc_end->offset()); 
  }

  void seek(Offset offset) override {
        
    // 1. Seek all the included ISRs to the first occurrence beginning at
    // the target location.
    for (auto &stream : include) {
      stream->seek(offset);
      if (stream->is_end()) return;
    }

    while (1) {

      // 2. Move the document end ISR to just past the furthest
      // contained ISR, then calculate the document begin location.
      Offset farthest = 0;
      for (auto stream : include) {
        farthest = max(farthest, stream->offset());
      }
      doc_end->seek(farthest + 1);
      if (doc_end->is_end()) return;
      const auto doc_begin = doc_end->offset() - doc_end->len();

      // 3. Seek all the other contained terms to past the document begin.
      bool good = true;
      for (auto stream : include) {
        stream->seek(doc_begin);

        // 5. If any ISR reaches the end, there is no match.
        if (stream->is_end()) return;

        // 4. If any contained erm is past the document end, return to
        // step 2.
        if (stream->offset() > doc_end->offset()) good = false;
      }

      if (good) break;
    }

    // 6. Seek all the excluded ISRs to the first occurrence beginning at
    // the document begin location.
    const auto doc_begin = doc_end->offset() - doc_end->len();
    for (auto stream : exclude) {
      if (stream->is_end()) continue;

      stream->seek(doc_begin);

      // 7. If any excluded ISR falls within the document, reset the
      // target to one past the end of the document and return to
      // step 1.
      if (stream->offset() < doc_end->offset()) {
        return seek(doc_end->offset());
      }
    }
  }

  Offset offset() override {
    return include[0]->offset();
  }

  bool is_end() override {
    for (auto stream : include) {
      if (stream->is_end()) return true;
    }
    return false;
  }

  ~isr_container() override {
    for (auto stream : include) delete stream;
    for (auto stream : exclude) delete stream;
    delete doc_end;
  }
};

// call seek 0 to init
class isr_phrase : public isr {

  vector<isr*> streams;

  public:

  void add_stream(isr *stream) {
    streams.push_back(stream);
  }

  void next() override {
    seek(offset() + 1);
  }

  void seek(Offset offset) override {
    for (auto stream : streams) {
      stream->seek(offset);
    }

    while (!streams.back()->is_end()) {
      const auto base = streams.back()->offset();
      bool good = true;
      for (auto i = 0u; i < streams.size() - 1 && good; ++i) {
        const auto goal = base - streams.size() + i + 1;
        streams[i]->seek(goal);

        if (streams[i]->is_end()) return;
        else if (streams[i]->offset() != goal) {
          streams.back()->next();
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
    for (auto stream : streams) {
      if (stream->is_end()) return true;
    }
    return false;
  }

  ~isr_phrase() override {
    for (auto stream : streams) delete stream;
  }
};

}
