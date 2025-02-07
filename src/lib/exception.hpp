#pragma once

#include "string.hpp"

namespace fast {
class exception {
public:
  explicit exception(const string &msg) : msg_(msg) {};

  const string &what() { return msg_; }

private:
  string msg_;
};
} // namespace fast
