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


  s.insert(0, '#');

  assert(s == "#cccc");

  assert(string("abs").ends_with("bs"));
  assert(string("abs").ends_with("abs"));
  assert(!string("abs").ends_with("aabs"));

  assert(string("multidimensional").ends_with("al"));

}
