#include <atomic>
#include <string.hpp>
#include <url_sender.hpp>
#include <semaphore>

using namespace fast::crawler;

const fast::string urls[] = {"www,google.com", "www.github.com", "www.walmart.com"};
std::atomic_int32_t idx = 0;

std::counting_semaphore<10> cs(0);

void test(const fast::string &url) {
  const auto tid = idx.fetch_add(1);
  assert(url == urls[tid]);
  cs.release();
}


int main(int argc, char **argv) {
  assert(argc == 2);
  url_sender sender(argv[1], test);
  for (const auto &url : urls) {
    sender.send_link(url);
  }
  sender.flush(0);
  cs.acquire();
  cs.acquire();
  cs.acquire();
  assert(idx.load() == 3);
}
