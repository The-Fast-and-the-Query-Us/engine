#include <pthread.h>
#include "../lib/bloom_filter.hpp"
#include "../lib/mutex.hpp"
#include "link_finder.hpp"
#include "frontier.hpp"

static constexpr int THREAD_COUNT = 20;
static constexpr int LINK_COUNT = 1000000; // ONE MILLION

class crawler {
public:
  crawler() : crawl_frontier(), visited_urls(),  {

  }

  void run() {
    for (auto &t : thread_pool) {
      if (pthread_create(&t, nullptr, worker, nullptr)) {
        throw std::runtime_error("pthread creation failed\n");
      }
    }

    for (auto &t: thread_pool) {
      pthread_join(t, nullptr);
    }

    // frontier.dump();
    // bloom_filter.dump();
  }

private:
  fast::bloom_filter visited_urls;
  frontier crawl_frontier;
  pthread_t thread_pool[THREAD_COUNT];
  fast::mutex bloom_mutex;
  fast::mutex frontier_mutex;
  // We also need an unordered map for robots.txt stuff

  static void* worker(void *arg) {
    while (true) {
      const char *url = frontier.next_link();
      link_finder html_scraper(url);
    }



    /* 
     * wrap whole thing in while (true)
     * or run it a set number of times so we can safely exit and blob the data
     *
     * frontier_mutex.lock();
     * auto link = frontier.next();
     * frontier_mutex.unlock();
     *
     * bloom_mutex.lock();
     * if (visited_urls.contains(link)) { 
     *   bloom_mutex.unlock();
     *   continue;
     * }
     * visited_urls.insert(link);
     * bloom_mutex.unlock();
     *
     * send request and get the html
     * we do actually need to parse the html to get the next links
     * We parse here and add all the links found to the frontier if it is not contained
     * We could also just do a brute force scan to find URLs specfically
     *
     * for (auto &new_link : new_links) {
     *   bloom_mutex.lock();
     *   if (bloom_mutex.contains(new_link)) { continue; }
     *   frontier_mutex.lock();
     *   frontier.insert(new_link);
     *   frontier_mutex.unlock();
     * }
     *
     * frontier_mutex.lock();
     * bloom_mutex.lock();
     * bloom_filter.dump();
     * frontier.dump();
     *
     * need to dump the robots.txt info
     * 
     */
  }

  /*static void find_links()
   * Have to use Linux file stuff from lecture
   */
};
