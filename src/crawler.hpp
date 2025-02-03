#include "lib/bloom_filter.hpp"
#include "lib/mutex.hpp"

class crawler {
public:
  crawler() : ___ {

  }

  void run() {

  }

  void worker();
private:
  fast::bloom_filter visited_urls;
  // frontier crawl_frontier

};
