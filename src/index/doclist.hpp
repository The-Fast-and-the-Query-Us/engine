#pragma once

#include "list.hpp"
#include <cstddef>
#include "hashtable.hpp"

namespace fast {

class doclist {
public:

  static size_t size_needed(const list<hashtable::doc> &docs) {
    (void) docs;
    return 0; 
  }

};

}
