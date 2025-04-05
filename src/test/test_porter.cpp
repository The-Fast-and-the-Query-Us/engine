#include <cassert>
#include <english.hpp>
#include <iostream>
#include <ostream>
#include <string.hpp>

int main() {
  fast::string md = "multidimensional";
  fast::english::porter_stem(md);
  std::cout << md.begin() << std::endl;
  assert(md == "multidimension");

  fast::string ch = "characterization";
  fast::english::porter_stem(ch);
  assert(ch == "character");
}
