#include <string.hpp>
#include <string_view.hpp>
#include <vector.hpp>

namespace fast {

class url_components {
 private:
  string_view domain;
  int depth;
  int length;
  bool valid;

  bool check_valid_protocol(string_view url, const char* protocol_end) {
    if (protocol_end == nullptr) {
      return false;
    }
    string_view protocol(url.begin(), protocol_end);

    if (protocol == "http" || protocol == "https" || protocol == "ftp") {
      return true;
    }
    return false;
  }

  bool check_valid_tld(string_view domain) {
    // TODO
    size_t i = domain.size() - 1;
    const char* end = domain.end() - 1;

    for (; i >= 0; --i) {
      if (*end == '.') {
        break;
      }
      --end;
    }
    if (i == -1) {
      return false;
    }

    string_view tld(end + 1, domain.end());

    if (tld == "gov" || tld == "edu") {
      return true;
    }
  }

 public:
  url_components(const string_view url) {
    length = url.size();
    valid = true;
    depth = 0;

    // Find protocol separator
    const char* protocol_end = url.find("://");

    if (!check_valid_protocol(url, protocol_end)) {
      valid = false;
      return;
    }

    const char* domain_start = protocol_end + 3;
    size_t url_len = url.size();
    const char* domain_end = domain_start;

    size_t i = domain_end - url.begin();

    // Find domain end
    for (; i < url_len; ++i) {
      if (*domain_end == '/' || *domain_end == '?' || *domain_end == '#') {
        break;
      }
      ++domain_end;
    }

    domain = string_view(domain_start, domain_end + 1);

    if (!check_valid_tld(domain)) {
      valid = false;
      return;
    }

    // Find path start
    const char* path_start = domain_end;
    if (i != url_len && *path_start == '/') {
      ++path_start;
      ++i;
    }

    // Count path depth
    bool not_empty = false;

    while (i < url_len && *path_start != '?' && *path_start != '#') {
      if (*path_start == '/') {
        if (not_empty) {
          ++depth;
        }
        not_empty = false;
      } else {
        not_empty = true;
      }
      ++path_start;
      ++i;
    }

    // Add last segment
    if (not_empty) {
      ++depth;
    }
  }
};
}  // namespace fast