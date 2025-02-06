#include <cassert>
#include <list.hpp>

int main() {
  fast::list<int> l;
  for (int i = 0; i < 10000; ++i) {
    l.push_back(i);
  }
  
  {
    int i = 0;
    for (auto it = l.begin(); it != l.end(); ++it, ++i) {
      assert(*it == i);
    }
  }

  {
    int i = 0;
    for (const auto num : l) assert(num == i++);
  }
}
