#pragma once

#include <cstddef>
#include <cstring>

namespace fast::crawler {

struct html_file {
  static constexpr size_t INITIAL_SIZE = 1'000'000;
  static constexpr double SIZE_MULTIPLIER = 1.1;

  char* html{};
  size_t cap{};
  size_t size_{};

  html_file(size_t new_cap = INITIAL_SIZE)
      : html(new char[new_cap]), cap(new_cap) {}

  ~html_file() { delete[] html; }

  void add(char* buffer, size_t bytes) {
    if (size_ + bytes >= cap) {
      size_t new_cap = cap * SIZE_MULTIPLIER;
      char* new_buffer = new char[new_cap];
      memcpy(new_buffer, html, cap); // should be size_?
      delete []html;

      html = new_buffer;
      cap = new_cap;
    }

    memcpy(html + size_, buffer, bytes);
    size_ += bytes;
  }

  size_t size() const { return size_; }
};

}  // namespace fast::crawler
