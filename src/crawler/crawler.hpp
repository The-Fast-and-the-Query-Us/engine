#pragma once

#include "bloom_filter.hpp"
#include "frontier.hpp"
#include "link_finder.hpp"
#include <hashtable.hpp>
#include <hashblob.hpp>
#include <pthread.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unordered_map>

static constexpr int THREAD_COUNT = 20;
static constexpr int LINK_COUNT = 1000000; // ONE MILLION
static constexpr const char *BLOOM_FILE_PATH = "bloom_filter_dump.dat";
static constexpr const char *FRONTIER_PATH = "fronter_dump.dat";
static constexpr size_t BLOOM_FILTER_NUM_OBJ = 1e6;
static constexpr double BLOOM_FILTER_FPR = 1e-4;
static constexpr size_t BLOB_THRESHOLD = 500'000;

class crawler {
public:
  crawler()
      : visited_urls(BLOOM_FILTER_NUM_OBJ, BLOOM_FILTER_FPR, BLOOM_FILE_PATH),
        crawl_frontier(FRONTIER_PATH),
        word_bank(new fast::hashtable) {}

  void run() {
    for (auto &t : thread_pool) {
      // Lambda used as worker is not static
      if (pthread_create(
              &t, nullptr,
              [](void *arg) -> void* {
                auto self = static_cast<crawler*>(arg);
                self->worker();
                return nullptr;
              },
              this)) {
        throw std::runtime_error("worker_thread creation failed\n");
      }
    }

    for (auto &t : thread_pool) {
      pthread_join(t, nullptr);
    }

    visited_urls.save();
    crawl_frontier.save();
    write_blob("", word_bank, bank_mtx);
    delete word_bank;
  }

  void shutdown() { shutdown_flag = 1; }

private:
  volatile sig_atomic_t shutdown_flag = 0;
  // Bloom filter and frontier are thread safe
  fast::crawler::bloom_filter<fast::string> visited_urls;
  fast::crawler::frontier crawl_frontier;
  fast::hashtable *word_bank;
  fast::mutex bank_mtx;
  /*std::unordered_map<fast::string, std::unordered_set<fast::string>>*/
  pthread_t thread_pool[THREAD_COUNT]{};
  // We also need a map for robots.txt stuff

  void *worker() {
    link_finder html_scraper;
    while (!shutdown_flag) {
      fast::string url = "";
      // TODO: How do we get our start list to the right computers?
      while (url == "") {
        url = crawl_frontier.next();
      }
      visited_urls.insert(url);
      html_scraper.parse_url(url.begin());
      fast::vector<fast::string> extracted_links = html_scraper.parse_html(*word_bank, bank_mtx);
      for (auto &link : extracted_links) {
        if (!visited_urls.contains(link)) {
          crawl_frontier.insert(link);
        }
      }
      crawl_frontier.notify_crawled(url);
      if (word_bank->tokens() > BLOB_THRESHOLD) {
        write_blob("", word_bank, bank_mtx);
      }
    }
    return nullptr;
  }

  void write_blob(const fast::string &path, fast::hashtable *word_bank, fast::mutex &bank_mtx) {
    bank_mtx.lock();
    const auto fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);

    if (fd == -1) {
      perror("open failed");
      exit(1);
    }

    size_t space = fast::hashblob::size_needed(*word_bank);

    if (ftruncate(fd, space) == -1) {
      perror("Ftruncate fail");
      exit(1);
    }

    auto mptr = mmap(nullptr, space, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mptr == MAP_FAILED) {
      perror("cant mmap");
      close(fd);
      exit(1);
    }

    auto blob = static_cast<fast::hashblob*>(mptr);
    fast::hashblob::write(*word_bank, blob);

    delete word_bank;
    word_bank = new fast::hashtable;

    munmap(mptr, space);
    close(fd);
    bank_mtx.unlock();
  }

};
