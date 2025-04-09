#include <cassert>
#include <compress.hpp>
#include <cstdlib>

using namespace fast;

void test(uint64_t num) {
  const auto sz = encoded_size(num);
  auto buffer = new unsigned char[sz];
  assert(buffer + sz == encode(num, buffer));

  uint64_t read;
  assert(decode(read, buffer) == buffer + sz);

  assert(num == read);
  assert(skip_encoded(buffer) == buffer + sz);

  delete[] buffer;
}
int main() {
  test(0);
  srand(0);

  for (auto i = 0u; i < 100; ++i) {
    test(rand());
  }
}
