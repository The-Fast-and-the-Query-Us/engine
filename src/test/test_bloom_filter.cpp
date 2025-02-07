#include "bloom_filter.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  fast::bloom_filter<std::string> bf(100, 0.01);

  bf.insert("hello");

  std::cout << bf.contains("hello") << std::endl;
  std::cout << bf.contains("hell") << std::endl;

  return 0;
}
