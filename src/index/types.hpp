#pragma once

#include "compress.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "string.hpp"

namespace fast::index {

enum post_type {
  Text,
  Doc,
};

template<post_type>
struct post;

template<>
struct post<Text> {
  uint64_t offset;

  operator uint64_t() const { return offset; }

  post(uint64_t offset) : offset(offset) {}

  size_t size_required(uint64_t last) const {
    return encoded_size(offset - last);
  }

  unsigned char *write(unsigned char *buffer, uint64_t last) const {
    return encode(offset - last, buffer);
  }

  static size_t term_size() {
    return encoded_size(0);
  }

  static unsigned char *write_term(unsigned char *buff) {
    return encode(0, buff);
  }
};

template <>
struct post<Doc> : post<Text> {
  string url;

  post(const string_view &sv, uint64_t offset) : post<Text>(offset), url(sv) {}

  size_t size_required(uint64_t last) const {
    return post<Text>::size_required(last) + encoded_size(url.size()) + url.size();
  }

  unsigned char *write(unsigned char *buffer, uint64_t last) const {
    buffer = post<Text>::write(buffer, last);
    buffer = encode(url.size(), buffer);
    memcpy(buffer, url.begin(), url.size());
    return buffer + url.size();
  }

  static size_t term_size() {
    return post<Text>::term_size() + encoded_size(0);
  }

  static unsigned char *write_term(unsigned char *buff) {
    buff = post<Text>::write_term(buff);
    return encode(0, buff);
  }
};

template <post_type>
class isr;

template<>
class isr<Text> {
protected:

  uint64_t acc;
  const unsigned char *buff;

public:

  isr(const unsigned char *buff) : buff(buff) {}

  isr(const unsigned char *buff, uint64_t acc) {
    this->buff = decode(this->acc, buff);
    this->acc += acc;
  }

  uint64_t operator*() const { return acc; }
  operator uint64_t() const { return acc; }

  bool operator==(const isr &other) const {
    return buff == other.buff;
  }

  isr &operator++() {
    uint64_t tmp;
    buff = decode(tmp, buff);
    acc += tmp;

    return *this;
  }
};

template<>
class isr<Doc> : public isr<Text> {
  const unsigned char *word;

public:

  string_view url() const {
    return string_view((const char *)word, (const char *)buff);
  }

  isr(const unsigned char *buff) : isr<Text>(buff) {}

  isr(const unsigned char *buff, uint64_t acc) : isr<Text>(buff, acc) {
    uint64_t len;
    this->buff = decode(len, this->buff);
    word = this->buff;
    this->buff += len;
  }

  isr &operator++() {
    isr<Text>::operator++();
    uint64_t len;
    buff = decode(len, buff);
    word = buff;
    buff += len;
    return *this;
  }
};

}
