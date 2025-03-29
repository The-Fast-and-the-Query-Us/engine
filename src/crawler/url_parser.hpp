#pragma once

#include <netdb.h>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>

class url_parser {
  public:
    char *path_buffer, *service, *host, *port, *path;

    url_parser() = default;

    ~url_parser() { delete[] path_buffer; }

    void parse(const char *url) {
      path_buffer = new char[strlen(url) + 1];

      const char *f{};
      char *t{};
      for (t = path_buffer, f = url; *t++ = *f++;) {}

      service = path_buffer;

      const char COLON = ':';
      const char SLASH = '/';
      char *p{};
      for (p = path_buffer; *p && *p != COLON; p++) {}

      if (*p) {
        // Mark the end of the Service.
        *p++ = 0;

        if (*p == SLASH) { ++p; }
        if (*p == SLASH) { ++p; }

        host = p;

        for (; *p && *p != SLASH && *p != COLON; p++) {}

        if (*p == COLON) {
          // port specified.  Skip over the COLON and
          // the port number.
          *p++ = 0;
          port = +p;
          for (; *p && *p != SLASH; p++) {}
        } else {
          port = p;
        }

        if (*p) {
          // Mark the end of the host and port.
          *p++ = 0;
        }

        // Whatever remains is the path.
        path = p;
      } else {
        host = path = p;
      }
    }
};
