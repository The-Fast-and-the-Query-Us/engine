#include <cassert>
#include <compress.hpp>
#include <cstddef>
#include <cstdlib>

int main() {
  srand(0);
  
  char buffer[10];

  for (auto i = 0ul; i < 6000; ++i) {
    const auto num = rand();
    fast::compress(num, buffer);
    int n{};
    fast::expand(n, buffer);
    assert(n == num);
  }
}
