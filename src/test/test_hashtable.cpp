#include <cassert>
#include <hashtable.hpp>

using namespace fast;

int main() {
  hashtable ht;
  static_string strings[] = {"a", "b", "c", "d", "e"};

  for (auto i = 0u; i < 5; ++i) {
    for (auto j = 0u; j <= i; ++j)
      ht.add(strings[i], {0, j});
  }

  for (auto i = 0u; i < 5; ++i) {
    auto j = 0u;
    const auto ptr = ht.get(strings[i]);
    assert (ptr->posts.length() == i + 1);
    for (const auto p : ptr->posts) {
      assert(p.doc_id == 0);
      assert(p.offset == j++);
    }
  }
}
