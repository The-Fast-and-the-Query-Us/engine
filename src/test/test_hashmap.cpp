#include "string.hpp"
#include <cassert>
#include <hashmap.hpp>

using namespace fast;

string strings[] = {"abc", "bobby", "james lu", "abg warrior", "yer"};

int main() {
  hashmap<string, int> mp;

  for (const auto &str : strings) {
    assert(mp[str] == 0);
    mp[str] = 1;
    assert(mp[str] == 1);
  }

  assert(mp.size() == 5);
}
