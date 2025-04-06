#include <math.h>

#include <string.hpp>
#include <string_view.hpp>
#include <vector.hpp>

namespace fast {

// static ranking weight
class url_components {
 private:
  string_view domain;
  int depth;
  int length;
  double tld_mult;
  double proto_mult;
  double mult;

  // if protocol is valid
  bool check_valid_protocol(string_view url, const char* protocol_end) {
    if (protocol_end == nullptr) {
      proto_mult = 0.5;
      return false;
    }
    string_view protocol(url.begin(), protocol_end);

    if (protocol == "http" || protocol == "https" || protocol == "ftp") {
      proto_mult = 2.0;
      return true;
    }
    proto_mult = 0.75;
    return false;
  }

  bool check_valid_tld(string_view domain) {
    // check if top level domain is reputable
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
      tld_mult = 1.5;
    } else if (tld == "com" || tld == "org") {
      tld_mult = 1.25;
    } else {
      tld_mult = 1.0;
    }
  }

 public:
  double get_multiplier() const { return mult; }

  url_components(const string_view url) {
    length = url.size();
    depth = 0;

    // Find protocol separator
    const char* protocol_end = url.find("://");

    check_valid_protocol(url, protocol_end);

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

    check_valid_tld(domain);

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

    double depth_pen = 1.0 / (1.0 + 0.15 * depth);
    double length_pen = exp(-length / 100.0);

    mult = 0;
    mult += 0.1 * proto_mult;
    mult += 0.35 * tld_mult;
    mult += 0.35 * depth_pen;
    mult += 0.2 * length_pen;
  }
};
}  // namespace fast