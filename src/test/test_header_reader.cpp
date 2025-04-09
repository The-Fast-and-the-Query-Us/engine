#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <cassert>

const char *pattern = "content-type: text/html";
constexpr uint8_t PATTERN_LEN = 23;

bool check_data_is_html(const char *buffer, size_t bytes) {
  uint8_t pattern_index = 0;

  for (size_t i = 0; i < bytes; ++i) {
    if (buffer[i] == pattern[pattern_index]) {
      ++pattern_index;
      for (; pattern_index < PATTERN_LEN && i < bytes; ++pattern_index) {
        if (buffer[i + pattern_index] != pattern[pattern_index]) {
          i += pattern_index - 1;
          pattern_index = 0;
          break;
        }
        std::cout << buffer[i + pattern_index];
      }
      if (pattern_index == PATTERN_LEN) return true;
    }
  }

  return false;
}

int main() {
  std::string buf = "content-type: text/h";
  assert(!check_data_is_html(buf.data(), buf.size()));
  return 0;
}
