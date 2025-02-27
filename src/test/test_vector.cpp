#include <cassert>
#include <vector.hpp>

using namespace fast;

int main() {
  vector<int> v;

  for (auto i = 0u; i < 100; ++i) {
    v.push_back(i);
  }

  int i = 0;
  for (const auto num : v) assert(num == i++);

  assert(v.size() == 100);

  vector<vector<int>> d2;

  for (int i = 0; i < 5; ++i) {
    d2.push_back({});
  }

  for (int i = 0; i < 5; ++i) d2[i].push_back(i);

  i = 0;
  for (const auto &vex : d2) assert(vex[0] == i++);
}
