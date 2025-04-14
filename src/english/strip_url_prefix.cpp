#include <english.hpp>

fast::string_view fast::english::strip_url_prefix(const fast::string& url) {
  fast::string_view stripped = url;
  if (url.starts_with("http://")) {
    stripped.trim_prefix(7);
  } else if (url.starts_with("https://")) {
    stripped.trim_prefix(8);
  }

  if (stripped.starts_with("www.")) {
    stripped.trim_prefix(4);
  }

  return stripped;
}

