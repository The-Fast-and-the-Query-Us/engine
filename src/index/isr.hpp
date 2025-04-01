#pragma once

#include "hashtable.hpp"
#include <compress.hpp>

namespace fast {

class isr {
  public:
  virtual void next();
  virtual void seek(Offset offset);
  virtual Offset offset();
  virtual bool has_next();
};

class isr_word : public isr {
  private:

  Offset last;
  Offset acc;

  const unsigned char *base;
  const pair<size_t> *sync_start, *sync_end;

  protected:

  const unsigned char *buff;

  public:
  isr_word(Offset last, const unsigned char *base, pair<size_t> *sync_start, pair<size_t> *sync_end) : 
    last(last), acc(0), base(base), sync_start(sync_start), sync_end(sync_end), buff(base) {};
  
  void next() override {
    uint64_t tmp;
    buff = decode(tmp, buff);
    acc += tmp;
  }

  void seek(Offset offset) override {
    while (sync_start != sync_end && sync_start->second <= offset) {
      acc = sync_start->second;
      buff = base + sync_start->first;
      ++sync_start;
    }

    while (acc < last && acc <= offset) next();
  }

  Offset offset() override {
    return acc;
  }

  bool has_next() override {
    return acc < last;
  }
};

class isr_doc : public isr_word {

  Offset doc_len;
  Offset url_len;
  const char *url;

public:
  isr_doc(Offset last, const unsigned char *base, pair<size_t> *sync_start, pair<size_t> *sync_end) : 
    isr_word(last, base, sync_start, sync_end) {}

  Offset len() const { return doc_len; }

  string_view get_url() const { return {url, url_len}; }

  void next() override {
    isr_word::next();

    uint64_t tmp;
    buff = decode(tmp, buff);
    doc_len = tmp;
    buff = decode(tmp, buff);
    url_len = tmp;
    url = (const char *)buff;
    buff += url_len;
  }
};

}
