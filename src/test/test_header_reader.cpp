#include <iostream>
//#include <cstdint>
#include <cstring>
#include <string>
#include <cassert>
#include "communicator.hpp"

// const char *pattern = "content-type: text/html";
// constexpr uint8_t PATTERN_LEN = 23;

int main() {
  std::string words[10] = {
    "",
    "abc",
    "xyzabc",
    "xyz abc xyz",
    "ababc",
    "AbC",
    "abcabcabc",
    "ab",
    "aabc",
    "abababcdef"
  };
  for (const auto &word : words) {
    std::cout << word << ": " <<
      fast::crawler::communicator::find_pattern_in_buffer(word.data(), word.size(), "abc", 3)
      << '\n';
  }
  return 0;
}
