#pragma once

#include <hashtable.hpp>
#include <post_list.hpp>

namespace fast::query {

class isr {
public:
  virtual void next();
  virtual bool has_next();
  virtual void seekf(Offset offset);
  virtual Offset offset();
};

class isr_word : isr {
  const post_list *pl;

  public:
  isr_word(const post_list *pl) : pl(pl) {}
};

}
