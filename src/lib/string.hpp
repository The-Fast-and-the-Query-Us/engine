// string.h
//
// Starter file for a string template

#pragma once

#include <cstddef>  // for size_t
#include <iostream> // for ostream

namespace fast {

class string {
public:
  // Default Constructor
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Creates an empty string
  string() : len_{0}, cap_{0} { buf_ = (char *)malloc(1); }

  // string Literal / C string Constructor
  // REQUIRES: cstr is a null terminated C style string
  // MODIFIES: *this
  // EFFECTS: Creates a string with equivalent contents to cstr
  string(const char *cstr) : len_{0}, cap_{0}, buf_{nullptr} {
    size_t len{0};

    for (auto ptr = cstr; *ptr; ++ptr, ++len)
      ;

    grow(len);

    for (size_t i = 0; i < len; ++i)
      buf_[i] = cstr[i];

    len_ = len;
  }

  // Size
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns the number of characters in the string
  size_t size() const { return len_; }

  // C string Conversion
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns a pointer to a null terminated C string of *this
  const char *cstr() const {
    buf_[len_] = 0;
    return buf_;
  }

  // Iterator Begin
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns a random access iterator to the start of the string
  const char *begin() const { return buf_; }

  // Iterator End
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns a random access iterator to the end of the string
  const char *end() const { return buf_ + len_; }

  // Element Access
  // REQUIRES: 0 <= i < size()
  // MODIFIES: Allows modification of the i'th element
  // EFFECTS: Returns the i'th character of the string
  char &operator[](size_t i) { return buf_[i]; }

  // string Append
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Appends the contents of other to *this, resizing any
  //      memory at most once
  void operator+=(const string &other) {
    auto new_len = len_ + other.len_;

    grow(new_len);

    for (size_t i = 0; i < other.len_; ++i) {
      buf_[i + len_] = other.buf_[i];
    }

    len_ += other.len_;
  }

  // Push Back
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Appends c to the string
  void pushBack(char c) {
    if (len_ == cap_)
      expand();

    buf_[len_] = c;
    ++len_;
  }

  // Pop Back
  // REQUIRES: string is not empty
  // MODIFIES: *this
  // EFFECTS: Removes the last charater of the string
  void popBack() { --len_; }

  // Equality Operator
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns whether all the contents of *this
  //    and other are equal
  bool operator==(const string &other) const { return this->cmp(other) == 0; }

  // Not-Equality Operator
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns whether at least one character differs between
  //    *this and other
  bool operator!=(const string &other) const { return this->cmp(other) != 0; }

  // Less Than Operator
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns whether *this is lexigraphically less than other
  bool operator<(const string &other) const { return this->cmp(other) == -1; }

  // Greater Than Operator
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns whether *this is lexigraphically greater than other
  bool operator>(const string &other) const { return this->cmp(other) == 1; }

  // Less Than Or Equal Operator
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns whether *this is lexigraphically less or equal to other
  bool operator<=(const string &other) const { return this->cmp(other) != 1; }

  // Greater Than Or Equal Operator
  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns whether *this is lexigraphically less or equal to other
  bool operator>=(const string &other) const { return this->cmp(other) != -1; }

  // < => -1
  // == => 0           green fn
  // > => 1
  int cmp(const string &other) const {
    size_t end = (len_ < other.len_) ? len_ : other.len_;
    for (size_t i = 0; i < end; ++i) {
      if (buf_[i] < other.buf_[i])
        return -1;
      else if (buf_[i] > other.buf_[i])
        return 1;
    }

    if (len_ == other.len_)
      return 0;
    else if (len_ < other.len_)
      return -1;
    else
      return 1;
  }

private:
  size_t len_;
  size_t cap_;
  char *buf_;

  void grow(size_t new_cap) {
    if (new_cap <= cap_)
      return;

    auto new_buf = (char *)malloc(new_cap + 1);

    for (size_t i = 0; i < len_; ++i)
      new_buf[i] = buf_[i];

    free(buf_);
    buf_ = new_buf;
    cap_ = new_cap;
  }

  void expand() {
    size_t new_len = ((cap_ << 1) < 8) ? 8 : cap_ << 1;
    grow(new_len);
  }
};

std::ostream &operator<<(std::ostream &os, const string &s) {
  return os << s.cstr();
}
} // namespace fast
