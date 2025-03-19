#include <cassert>
#include <string.hpp>

using namespace fast;

int main() {
  string s;

  assert(s.size() == 0);

  s += 'c';
  assert (s.size() == 1);

  const auto other = s;
  assert(other == s);

  s += other;
  s += s;
  
  assert(s == "cccc");

  assert(string("some") == string_view("some"));
}
