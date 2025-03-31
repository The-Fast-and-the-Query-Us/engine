#pragma once

#include "string.hpp"
#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace fast {

inline bool send_all(const int fd, const char *data, size_t len) {
  while (len > 0) {
    const auto sent = send(fd, data, len, 0);
    if (sent < 0) return false;
    data += sent;
    len -= sent;
  }
  return true;
}

inline bool send_all(const int fd, uint32_t number) {
  number = htonl(number);
  return send_all(fd, reinterpret_cast<const char *>(&number), sizeof(number));
}

// send string as <length><data>
inline bool send_all(const int fd, const string &str) {
  if (!send_all(fd, str.size())) return false;
  if (!send_all(fd, str.begin(), str.size())) return false;
  return true;
}

}
