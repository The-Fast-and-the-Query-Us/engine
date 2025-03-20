#pragma once

#include "../lib/bloom_filter.hpp"
#include "frontier.hpp"
#include "link_finder.hpp"
#include <pthread.h>
#include <stdexcept>
#include <unordered_map>

static constexpr int THREAD_COUNT = 20;
static constexpr int LINK_COUNT = 1000000; // ONE MILLION
static constexpr const char *BLOOM_FILE_PATH = "bloom_filter_dump.dat";
static constexpr size_t BLOOM_FILTER_NUM_OBJ = 1e6;
static constexpr double BLOOM_FILTER_FPR = 1e-4;

class crawler {
public:
  crawler()
      : visited_urls(BLOOM_FILTER_NUM_OBJ, BLOOM_FILTER_FPR, BLOOM_FILE_PATH),
        crawl_frontier(/*file_path*/) {}

  void run() {
    if (pthread_create(
            &blob_thread, nullptr,
            [](void *arg) -> void * {
              auto self = static_cast<crawler *>(arg);
              self->worker();
              return nullptr;
            },
            this)) {
      throw std::runtime_error("blob_thread creation failed\n");
    }

    for (auto &t : thread_pool) {
      // Lambda used as worker is not static
      if (pthread_create(
              &t, nullptr,
              [](void *arg) -> void * {
                auto self = static_cast<crawler *>(arg);
                self->worker();
                return nullptr;
              },
              this)) {
        throw std::runtime_error("pthread creation failed\n");
      }
    }

    for (auto &t : thread_pool) {
      pthread_join(t, nullptr);
    }
    pthread_join(blob_thread, nullptr);
  }

  void shutdown() { shutdown_flag = 1; }

private:
  volatile sig_atomic_t shutdown_flag = 0;
  // Bloom filter and frontier are thread safe
  fast::bloom_filter<fast::string> visited_urls;
  fast::crawler::frontier crawl_frontier;
  /*std::unordered_map<fast::string, std::unordered_set<fast::string>>*/
  pthread_t blob_thread{};
  pthread_t thread_pool[THREAD_COUNT]{};
  // We also need a map for robots.txt stuff

  void *blobber(void *arg) {
    auto self = static_cast<crawler *>(arg);
    struct timespec sleep_time{};
    sleep_time.tv_sec = 10;
    sleep_time.tv_nsec = 0;

    while (!self->shutdown_flag) {
      nanosleep(&sleep_time, nullptr);

      if (!self->shutdown_flag) {
        visited_urls.save();
        crawl_frontier.save();
      }
    }

    return nullptr;
  }

  void *worker() {
    link_finder html_scraper;
    while (!shutdown_flag) {
      fast::string url = "";
      while (url == "") {
        url = crawl_frontier.next();
      }
      visited_urls.insert(url);
      html_scraper.parse_url(url.begin());
      fast::vector<fast::string> extracted_links = html_scraper.extract_links();
      for (auto &link : extracted_links) {
        if (!visited_urls.contains(link)) {
          crawl_frontier.insert(link);
        }
      }
      crawl_frontier.notify_crawled(url);
    }
    return nullptr;
  }
};
