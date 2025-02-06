#include <cassert>

#include <static_string.hpp>

using namespace fast;

int main() {
  static_string ss;
  static_string other("words");
  ss = other;

  static_string another(other);

  assert(ss == another);
  assert(ss == "words");
}
