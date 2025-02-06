#include <common.hpp>
#include <cassert>

using namespace fast;

int main() {
  tuple<int, int , int> tp(1, 2, 3);

  assert(get<0>(tp) == 1);
}
