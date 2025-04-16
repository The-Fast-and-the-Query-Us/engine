#include "string_view.hpp"
#include <cassert>

using namespace fast;

int main() {
  string_view sv;
  assert(sv == "");
  assert(sv != "a");

  string_view me("bobby");
  assert(me == "bobby");

  assert(sv < me);
  assert(sv < "bobby");

  sv = me;
  assert(sv == me);

  string_view other("other");
  assert(other > me);
  assert(other > "bobbyajksd");

  assert(other.contains("ot"));
  assert(other.contains("th"));
  assert(!other.contains("z"));
}
