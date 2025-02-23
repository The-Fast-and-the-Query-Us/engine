#include <cassert>
#include <cstdlib>
#include <hashblob.hpp>
#include <hashtable.hpp>

using namespace fast;

int main() {
  hashtable ht;
  static_string strings[] = {"a", "b", "c", "d", "asdfghjkl"};

  for (const auto &s : strings) {
    ht.add(s, {0, 0});
  }

  const auto opts = hashblob::get_opts(&ht);


  auto buffer = (hashblob*) new size_t[opts.file_size / sizeof(size_t) + 1];

  hashblob::write(&ht, buffer, &opts);

  for (const auto &s : strings) {
    assert(buffer->get_posts(s) != nullptr);
  }

  static_string ss("smth random");
  assert(buffer->get_posts(ss) == nullptr);

  delete[] buffer;
}
