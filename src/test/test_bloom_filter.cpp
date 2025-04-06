#include <cassert>
#include <iostream>
#include "bloom_filter.hpp"
#include "string.hpp"

int main() {
  fast::crawler::bloom_filter<fast::string> bf(100, 0.01);

  bf.insert("hello");

  assert(bf.contains("hello"));
  assert(!bf.contains("hell"));
  std::cout << "PASS string basic" << std::endl << std::endl;
}
