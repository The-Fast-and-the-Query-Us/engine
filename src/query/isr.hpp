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

  isr *stream;
  size_t count;

  public:

  void next() override {

  }

  void seek(Offset offset) override {

  }

  Offset offset() override {
    return 0;
  }
  
  bool is_end() override {

  }
};

class isr_and : public isr {

};

class isr_phrase : public isr {
  isr *stream;
  size_t count;

  public:

};

}
