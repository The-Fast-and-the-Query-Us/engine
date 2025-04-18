#include <bitset_opt.hpp>
#include <cassert>
#include <iostream>

int main() {
  size_t n = 100;
  fast::bitset_opt bs(n);

  // set different bits
  for (size_t i = 0; i < bs.size(); ++i) {
    bs.set(i, i % 2);
  }
  // check bits
  for (size_t i = 0; i < bs.size(); ++i) {
    assert(static_cast<bool>(bs.test(i)) == (i % 2));
  }

  // set bits to the same thing to make sure they don't flip
  for (size_t i = 0; i < bs.size(); ++i) {
    bs.set(i, i % 2);
  }
  // check bits
  for (size_t i = 0; i < bs.size(); ++i) {
    assert(static_cast<bool>(bs.test(i)) == (i % 2));
  }

  // check assignment operator and destructor
  {
    fast::bitset_opt temp;
    temp = bs;
    // check bits of the temp;
    for (size_t i = 0; i < bs.size(); ++i) {
      assert(static_cast<bool>(bs.test(i)) == (i % 2));
    }
  }

  // check int assignment operator
  bs.set(3, 0);
  assert(bs.test(3) == 0);

  // check equality operators
  assert(bs.test(0) != bs.test(1));
  assert(bs.test(0) == bs.test(2));

  assert(bs.test(0) != true);
  assert(bs.test(1) == true);

  std::cout << bs.num_chunks << '\n';
  std::cout << "PASS" << '\n';
}
