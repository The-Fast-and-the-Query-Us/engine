#pragma once
#include <cstddef>
namespace fast {

class dictionary {
  size_t num_buckets;
  public:
  
  static size_t size_needed();

  static char *write();
};

}
