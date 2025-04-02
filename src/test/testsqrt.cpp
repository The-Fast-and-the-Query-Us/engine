#include <cassert>
#include <common.hpp>

int main(void) {

  for (uint64_t i = 1; i < 5000; ++i) {
    assert(fast::fast_sqrt(i * i) == i);
    assert(fast::fast_sqrt(i * i + 1) == i);
    assert(fast::fast_sqrt(i * i - 1) == i - 1);
  }
}
