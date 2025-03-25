#include <cassert>
#include <compress.hpp>
#include <cstdlib>

using namespace fast;

unsigned char buffer[20];

template<EncodeMethod em>
void test(uint64_t num) {
  const auto sz = encoded_size<em>(num);
  assert(buffer + sz == encode<em>(num, buffer));

  uint64_t read;
  assert(decode<em>(read, buffer) == buffer + sz);

  assert(num == read);
  assert(skip_encoded<em>(buffer) == buffer + sz);
}

template<EncodeMethod em>
void run_test() {
  test<em>(0);
  srand(0);
  for (auto i = 0u; i < 400; ++i) {
    test<em>(rand());
  }
}

int main() {
  run_test<EncodeMethod::Simple3>();
  run_test<EncodeMethod::Utf>();
}
