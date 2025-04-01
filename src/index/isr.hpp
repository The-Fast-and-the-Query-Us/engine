#pragma once

#include "hashtable.hpp"
#include <compress.hpp>
#include <cstdio>

namespace fast {

class isr {
  public:
  // return false if called at EOS
  virtual bool next() = 0;

  // return false if seek tried to read past EOS
  // stops at first offset > <offset>
  virtual bool seek(Offset offset) = 0;

  virtual Offset offset() = 0;

  virtual bool end(); // todo impl

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
    acc(0), end(base + len), base(base), sync_start(sync_start), sync_end(sync_end), buff(base) {};
  
  bool next() override {
    if (buff == end) 
      return false;

    uint64_t tmp;
    buff = decode(tmp, buff);
    acc += tmp;
    return true;
  }

  bool seek(Offset offset) override {
    while (sync_start != sync_end && sync_start->second <= offset) {
      if (sync_start->second > acc) {
        acc = sync_start->second;
        buff = base + sync_start->first;
      }
      ++sync_start;
    }

    while (buff != end && acc <= offset) next();
    return acc > offset;
  }

  Offset offset() override {
    return acc;
  }

  ~isr_word() override {}
};

// should initialize
class isr_doc : public isr_word {

  Offset doc_len;
  Offset url_len;
  const char *url;

public:
  isr_doc(Offset last, const unsigned char *base, const pair<size_t> *sync_start, const pair<size_t> *sync_end) : 
    isr_word(last, base, sync_start, sync_end) {}

  Offset len() const { return doc_len; }

  string_view get_url() const { return {url, url_len}; }

  bool next() override {
    if (!isr_word::next()) 
      return false;

    uint64_t tmp;
    buff = decode(tmp, buff);
    doc_len = tmp;
    buff = decode(tmp, buff);
    url_len = tmp;
    url = (const char *)buff;
    buff += url_len;

    return true;
  }
};

}
