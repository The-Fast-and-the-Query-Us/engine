#include <url_parser.hpp>
#include <string.hpp>
#include <pair.hpp>
#include <cassert>

using namespace fast;

struct t {
  string ans[3];
};

string tests[][3] = {
  {"https://example.com/a/b/c", ".", "https://example.com/a/b/"},
  {"https://example.com/a/b/c", "./d", "https://example.com/a/b/d"},
  {"https://example.com/a/b/c", "../d", "https://example.com/a/d"},
  {"https://example.com/a/b/c", "../../d", "https://example.com/d"},
  {"https://example.com/a/b/c", "../../../d", "https://example.com/d"},
  {"https://example.com/a/b/c", "../x/../y", "https://example.com/a/y"},
  {"https://example.com/dir/page.html", "about.html", "https://example.com/dir/about.html"},
  {"https://example.com/dir/", "about.html", "https://example.com/dir/about.html"},
  {"https://example.com/dir/", "subdir/", "https://example.com/dir/subdir/"},
  {"https://example.com/dir/page.html", "subdir/", "https://example.com/dir/subdir/"},
  {"https://example.com/dir/", "../index.html", "https://example.com/index.html"},
  {"https://example.com/dir/page.html", "../index.html", "https://example.com/index.html"},
  {"https://www.google.com", "https://github.com", "https://github.com"},
};

int main() {
  
  for (auto &p : tests) {
    assert(crawler::url_parser::url_join(p[0], p[1]) == p[2]);
  }
}
