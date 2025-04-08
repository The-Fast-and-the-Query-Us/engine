#include <cctype>
#include <english.hpp>

void fast::english::normalize_text(fast::string &word) {
  for (auto &c : word) {
    c = tolower(c);
  }

  while (word.size() > 0) {
    switch (word.back()) {
      case '.':
      case '!':
      case '?':
      case ',':
      case ')':
      case '(':
      case '#':
        word.pop_back();
      default:
        break;
    }
  }

  size_t ptr = 0;
  while (ptr < word.size()) {
    switch (word[ptr]) {
      case '.':
      case '!':
      case '?':
      case ',':
      case ')':
      case '(':
      case '#':
        ++ptr;
      default:
        break;
    }
  }

  word = word.substr(ptr, word.size() - 1);
}
