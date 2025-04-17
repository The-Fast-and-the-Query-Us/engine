#pragma once

#include "string.hpp"
#include "vector.hpp"
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

  static string normalize_path(const string &path) {
    string res;

    for (const auto c : path) {
      res += c;

      if (res.ends_with("/../")) {

        res.pop_back(4);
        while (res.size() > 0 && res.back() != '/') res.pop_back();

        if (res.size() == 0) res += '/';

      } else if (res.ends_with("/./")) {
        res.pop_back(2);
      }
    }

    if (res.back() == '.') res.pop_back();

    return res;
  }

  static string get_base_path(const string &base) {
    ssize_t l = -1;

    for (size_t i = 0; i < 3; ++i) {
      l = base.find('/', l + 1);

      if (l < 0) return "/";
    }

    return string(base.begin() + l);
  }

  /*
   * Get the root domain without trailing slash
   */
  static string get_base_root(const string &base) {
    ssize_t last = -1;
    for (size_t i = 0; i < 3; ++i) {
      last = base.find('/', last + 1);

      if (last < 0) return base;
    }

    return string(base.begin(), last);
  }

  /*
  * Remove /../ and /./ from a link
  */
  static string url_join(const string &base, const string &link) {
    if (link.starts_with("http://") || link.starts_with("https://")) {
      return link;
    }

    // protocol relative
    if (link.starts_with("//")) {
      size_t ptr = 0;
      while (ptr < base.size() && base[ptr] != '/') ++ptr;

      if (ptr == base.size()) {
        return string("http://") + link;
      } else {
        return base.substr(0, ptr) + link;
      }
    }

    if (link.size() > 0 && link[0] == '/') {
      return get_base_root(base) + normalize_path(link);
    }

    auto bp = get_base_path(base);
    while (bp.size() > 0 && bp.back() != '/') bp.pop_back();
    auto comp = bp + link;

    return get_base_root(base) + normalize_path(comp);
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
