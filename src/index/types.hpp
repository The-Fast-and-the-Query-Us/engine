#pragma once

#include "string_view.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace fast::index {

enum post_type {
  Title,
  Body,
  Doc,
};

class post {
  uintptr_t data;

  public:
  uint64_t offset;

  post(uint64_t offset, post_type pt) : data(1 | (pt << 1)), offset(offset) {}

  post(uint64_t offset, const string_view &sv) : offset(offset) {
    auto buffer = (char*) malloc(sv.size() + 1);
    memcpy(buffer, sv.begin(), sv.size());
    buffer[sv.size()] = 0;
    data = reinterpret_cast<uintptr_t>(buffer);
  }

  post_type get_type() {
    if (data & 1) return post_type(data >> 1);
    return Doc;
  }

  // must be post_type doc
  const char *get_url() const {
    return reinterpret_cast<char*>(data);
  }

  ~post() {
    if (get_type() == Doc) delete[] reinterpret_cast<char*>(data);
  }
};

}
