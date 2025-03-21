#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <bit>

namespace fast {

class svo_string {

  struct heap_t {
    char  *data;
    size_t cap;
    size_t len;
  };

  struct svo_t {
    unsigned char len : 7;
    unsigned char is_short : 1;
    char data[sizeof(heap_t) - 1];
  };

  static_assert(sizeof(heap_t) == sizeof(svo_t), "Size is incorrect");
  static_assert(std::endian::native == std::endian::little, "Svo requires little endian");

  union data_t {
    heap_t heap;
    svo_t svo;
  };

  data_t data;

  static constexpr size_t MAX_SVO_LEN = sizeof(svo_t::data) - 1;

public:
  svo_string() {
    data.svo.is_short = 1;
    data.svo.len      = 0;
    data.svo.data[0]  = 0;
  }

  ~svo_string() {
    if (!data.svo.is_short) {
      free(data.heap.data);
    }
  }

  void operator+=(char c) {
    if (data.svo.is_short && data.svo.len < MAX_SVO_LEN) {
      data.svo.data[data.svo.len] = c;
      data.svo.len++;
      data.svo.data[data.svo.len] = 0;
    } else {
      if (data.svo.is_short) {
        // TODO
      }
    }
  }

  size_t size() const {
    if (data.svo.is_short) {
      return data.svo.len;
    } else {
      return data.heap.len;
    }
  }

  char *begin() {
    if (data.svo.is_short) {
      return data.svo.data;
    } else {
      return data.heap.data;
    }
  }

  const char *begin() const {
    if (data.svo.is_short) {
      return data.svo.data;
    } else {
      return data.heap.data;
    }
  }

  char *end() {
    if (data.svo.is_short) {
      return data.svo.data + data.svo.len;
    } else {
      return data.heap.data + data.heap.len;
    }
  }

  const char *end() const {
    if (data.svo.is_short) {
      return data.svo.data + data.svo.len;
    } else {
      return data.heap.data + data.heap.len;
    }
  }

  char &operator[](size_t idx) {
    return begin()[idx];
  }

  const char &operator[](size_t idx) const {
    return begin()[idx];
  }
};

}
