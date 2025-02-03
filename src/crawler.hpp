#include "lib/bloom_filter.hpp"
#include "lib/mutex.hpp"

static constexpr int THREAD_COUNT = 20;

class crawler {
public:
  /*crawler() {*/
  /**/
  /*}*/

  void run() {
    for (auto &t : thread_pool) {
      if (pthread_create(&t, nullptr, worker, nullptr)) {
        throw std::runtime_error("pthread creation failed\n");
      }
    }

    for (auto &t: thread_pool) {
      pthread_join(t, nullptr);
    }
  }

private:
  // fast::bloom_filter visited_urls;
  // frontier crawl_frontier;
  pthread_t thread_pool[THREAD_COUNT];
  fast::mutex bloom_mutex;
  fast::mutex frontier_mutex;

  static void* worker(void *arg) {
    /* 
     * frontier_mutex.lock();
     * auto link = frontier.next();
     * frontier_mutex.unlock();
     *
     * bloom_mutex.lock();
     * if (visited_urls.contains(link)) { continue; }
     */

  }
};
