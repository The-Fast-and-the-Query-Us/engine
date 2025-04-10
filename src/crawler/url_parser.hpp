#pragma once

#include <netdb.h>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>

namespace fast::crawler {

class url_parser {
  char *path_buffer;

public:
  const char *complete_url;
  char *service, *host, *port, *path;

  url_parser(const char *url) : path_buffer(new char[strlen(url) + 1]), complete_url(url) {
    const char *f{};
    char *t{};
    for (t = path_buffer, f = url; (*t++ = *f++););

    service = path_buffer;

    const char COLON = ':';
    const char SLASH = '/';
    char *p{};
    for (p = path_buffer; *p && *p != COLON; ++p);

    if (*p) {
      // Mark the end of the service.
      *p++ = 0;

      if (*p == SLASH) ++p;
      if (*p == SLASH) ++p;

      host = p;

      for (; *p && *p != SLASH && *p != COLON; ++p);

      if (*p == COLON) {
        // Port specified.  Skip over the colon and
        // the port number.
        *p++ = 0;
        port = p;
        for (; *p && *p != SLASH; p++);
      } else {
        port = p;
      }

      if (*p) *p++ = 0;

      // Whatever remains is the Path.
      path = p;
    } else {
      host = path = p;
    }
  }

  ~url_parser() {
    delete[] path_buffer;
    complete_url = nullptr;
    service = nullptr;
    host = nullptr;
    port = nullptr;
    path = nullptr;
  }
};

}
