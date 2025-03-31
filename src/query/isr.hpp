#pragma once

#include <hashtable.hpp>
#include <post_list.hpp>

namespace fast::query {

class isr {
public:
  virtual void next();
  virtual void next_document();
  virtual void seek(Offset offset);
  virtual Offset offset();
  virtual bool end();
};

// figure out which methods we need
class isr_or : public isr {
  isr **terms;
  unsigned term_count;

public:

  void next() override {

  }

  void next_document() override {

  }

  void seek(Offset offset) override {

  }

  Offset offset() override {

  }

  bool end() override {

  }
};

}
