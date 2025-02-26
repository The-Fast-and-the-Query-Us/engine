#include <cassert>
#include <string.hpp>

using namespace fast;

int main() {
  string s;

  assert(s.length() == 0);

  s += 'c';
  assert (s.length() == 1);

  const auto other = s;
  assert(other == s);

  s += other;
  s += s;
  
  assert(s == "cccc");
}
