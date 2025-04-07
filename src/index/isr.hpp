#pragma once

#include "hashtable.hpp"
#include <compress.hpp>
#include <cstdio>

namespace fast {

class isr {
  public:

  // jump forward one post
  virtual void next() = 0;

  // jump to first post >= offset
  // or stop at end. NOTE: this only seeks
  // forward and wont do anything for 
  // backwards seek
  virtual void seek(Offset offset) = 0;

  // current offset of isr
  // call is_end to make sure this is valid
  virtual Offset offset() = 0;

  // returns true if isr was 'nexted' or seeked past the 
  // last occurence
  virtual bool is_end() = 0; // todo impl

  virtual ~isr(){};
};

class isr_word : public isr {
  private:

  Offset acc;
  const unsigned char *end;
  const unsigned char *base;
  const pair<size_t> *sync_start, *sync_end;

  protected:

  const unsigned char *buff;

  public:

  isr_word(size_t len, const unsigned char *base, const pair<size_t> *sync_start, const pair<size_t> *sync_end) : 
    acc(0), end(base + len), base(base), sync_start(sync_start), sync_end(sync_end), buff(base) {
    uint64_t tmp;
    buff = decode(tmp, buff);
    acc += tmp;
  };
  
  void next() override {
    uint64_t tmp;
    buff = decode(tmp, buff);
    acc += tmp;
  }

  void seek(Offset offset) override {
    while (sync_start != sync_end && sync_start->second < offset) {
      if (sync_start->second > acc) {
        acc = sync_start->second;
        buff = base + sync_start->first;
      }
      ++sync_start;
    }

    while (buff != end && acc < offset) next();
  }

  Offset offset() override {
    return acc;
  }

  bool is_end() override {
    return buff == end;
  }

  ~isr_word() override {}
};

// should initialize
class isr_doc : public isr_word {

  Offset doc_len;
  Offset url_len;
  const char *url;

  void next_impl() {
    if (!is_end()) {
      uint64_t tmp;
      buff = decode(tmp, buff);
      doc_len = tmp;
      buff = decode(tmp, buff);
      url_len = tmp;
      url = (const char *)buff;
      buff += url_len;
    }
  }

public:
  isr_doc(Offset last, const unsigned char *base, const pair<size_t> *sync_start, const pair<size_t> *sync_end) : 
    isr_word(last, base, sync_start, sync_end) {
    next_impl();
  }

  Offset len() const { return doc_len; }

  string_view get_url() const { return {url, url_len}; }

  void next() override {
    isr_word::next();
    next_impl();
  }
};

}
