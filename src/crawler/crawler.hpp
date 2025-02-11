#pragma once

#include <pthread.h>
#include "../lib/bloom_filter.hpp"
#include "link_finder.hpp"
#include "frontier.hpp"

static constexpr int THREAD_COUNT = 20;
static constexpr int LINK_COUNT = 1000000; // ONE MILLION
static constexpr const char *BLOOM_FILE_PATH = "bloom_filter_dump.dat";
static constexpr size_t BLOOM_FILTER_NUM_OBJ = 1e6;
static constexpr double BLOOM_FILTER_FPR = 1e-4;

class crawler {
public:
  crawler() : visited_urls(BLOOM_FILTER_NUM_OBJ, BLOOM_FILTER_FPR, BLOOM_FILE_PATH), crawl_frontier(/*file_path*/) {

  }

  void run() {
    for (auto &t : thread_pool) {
      // Lambda used as worker is not static
      if (pthread_create(&t, nullptr,
        [](void* arg) -> void* {
          auto self = static_cast<crawler*>(arg);
          self->worker();
          return nullptr;
        }, this)) {
        throw std::runtime_error("pthread creation failed\n");
      }
    }

    for (auto &t: thread_pool) {
      pthread_join(t, nullptr);
    }
  }

private:
  // Bloom filter and frontier are thread safe
  fast::bloom_filter<fast::string> visited_urls;
  fast::crawler::frontier crawl_frontier;
  pthread_t thread_pool[THREAD_COUNT]{};
  // We also need a map for robots.txt stuff

  void* worker() {
    while (true) {
      fast::string url = crawl_frontier.next();
      link_finder html_scraper(url.data());
      fast::vector<fast::string> extracted_links = html_scraper.extract_links();
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
};
