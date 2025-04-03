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

// send string as <len><data>
inline bool send_all(const int fd, const string &str) {
  if (!send_all(fd, str.size())) return false;
  if (!send_all(fd, str.begin(), str.size())) return false;
  return true;
}

inline bool recv_all(const int fd, uint32_t &number) {
  if (recv(fd, &number, sizeof(number), MSG_WAITALL) != sizeof(number)) return false;
  number = ntohl(number);
  return true;
}

inline bool recv_all(const int fd, string &str) {
  uint32_t str_len;
  if (!recv_all(fd, str_len)) return false;
  str.resize(str_len);
  if (recv(fd, str.begin(), str_len, MSG_WAITALL) == -1) return false;
  return true;
}

}
