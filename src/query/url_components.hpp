#include <string.hpp>
#include <vector.hpp>

namespace fast {

const char* find(const char* word, const char* to_find) {
  if (!*to_find) return word;

  size_t needle_len = 0;
  while (to_find[needle_len]) needle_len++;

  for (size_t i = 0; word[i]; i++) {
    bool match = true;

    for (size_t j = 0; j < needle_len; j++) {
      if (!word[i + j] || word[i + j] != to_find[j]) {
        match = false;
        break;
      }
    }

    if (match) {
      return &word[i];  // found match
    }
  }

  return nullptr;  // no match found
}

class url_components {
 private:
  string_view domain;
  int depth;
  int length;
  bool valid;

  bool check_valid_protocol(const char* url_str, const char* protocol_end) {
    if (protocol_end == nullptr) {
      return false;
    }
    string_view protocol(url_str, protocol_end);

    if (protocol == "http" || protocol == "https" || protocol == "ftp") {
      return true;
    }
    return false;
  }

  bool check_valid_tld(string_view domain) {
    // TODO
  }

 public:
  url_components(const string_view& url) {
    length = url.size();
    valid = true;
    depth = 0;

    // Find protocol separator
    const char* url_str = url.c_str();
    const char* protocol_end = find(url_str, "://");

    if (!check_valid_protocol(url_str, protocol_end)) {
      valid = false;
      return;
    }

    const char* domain_start = protocol_end ? protocol_end + 3 : url_str;

    // Find domain end
    const char* domain_end = domain_start;
    while (*domain_end && *domain_end != '/' && *domain_end != '?' &&
           *domain_end != '#') {
      domain_end++;
    }

    domain = string_view(domain_start, domain_end - domain_start);

    if (!check_valid_tld(domain)) {
      valid = false;
      return;
    }

    // Find path start
    const char* path_start = domain_end;
    if (*path_start == '/') path_start++;

    // Count path depth
    bool not_empty = false;

    while (*path_start && *path_start != '?' && *path_start != '#') {
      if (*path_start == '/') {
        if (not_empty) {
          ++depth;
        }
        not_empty = false;
      } else {
        not_empty = true;
      }
      path_start++;
    }

    // Add last segment
    if (not_empty) {
      ++depth;
    }
  }
};
}  // namespace fast