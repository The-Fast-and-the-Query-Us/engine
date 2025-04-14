#include <atomic>
#include <string.hpp>
#include <url_sender.hpp>
#include <semaphore>
#include <html_parser.hpp>

using namespace fast::crawler;

std::counting_semaphore<10> cs(0);

void test(const fast::string &url) {
  assert(url == "james lu");
  cs.release();
}


int main(int argc, char **argv) {
  assert(argc == 2);

  url_sender<0> sender(argv[1], test);

  fast::vector<Link> fl;

  for (auto i = 0; i < 10; ++i) {
    fl.emplace_back("james lu");
  }

  sender.add_links(fl);

  for (auto i = 0; i < 10; ++i) {
    cs.acquire();
  }
}
