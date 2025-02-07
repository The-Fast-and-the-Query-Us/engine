#include <bitset.hpp>
#include <cassert>
#include <iostream>

int main() {
  size_t n = 100;
  fast::bitset bs(n);

  // set different bits
  for (size_t i = 0; i < bs.size(); ++i) {
    bs[i] = bool(i % 2);
  }
  // check bits
  for (size_t i = 0; i < bs.size(); ++i) {
    assert(static_cast<bool>(bs[i]) == (i % 2));
  }

  // set bits to the same thing to make sure they don't flip
  for (size_t i = 0; i < bs.size(); ++i) {
    bs[i] = bool(i % 2);
  }
  // check bits
  for (size_t i = 0; i < bs.size(); ++i) {
    assert(static_cast<bool>(bs[i]) == (i % 2));
  }

  // check assignment operator and destructor
  {
    fast::bitset temp;
    temp = bs;
    // check bits of the temp;
    for (size_t i = 0; i < bs.size(); ++i) {
      assert(static_cast<bool>(bs[i]) == (i % 2));
    }
  }

  // check int assignment operator
  bs[3] = 0;
  assert(bs[3] == 0);

  // check equality operators
  assert(bs[0] != bs[1]);
  assert(bs[0] == bs[2]);

  assert(bs[0] != true);
  assert(bs[1] == true);

  std::cout << "PASS" << std::endl;
}
