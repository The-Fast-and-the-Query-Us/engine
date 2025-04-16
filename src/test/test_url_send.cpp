#include <string.hpp>
#include <url_sender.hpp>
#include <html_parser.hpp>

using namespace fast::crawler;

volatile int i = 0;

void test(const fast::string &url) {
  assert(url == "https://james lu");
  i = i + 1;
}


int main(int argc, char **argv) {
  assert(argc == 2);

  url_sender sender(argv[1], test);

  fast::vector<Link> fl;

  for (auto i = 0; i < 10; ++i) {
    fl.emplace_back(fast::string("https://james lu"));
  }

  sender.add_links(fl);

  while (i != 10);

  std::cout << "Pass" << std::endl;
}
