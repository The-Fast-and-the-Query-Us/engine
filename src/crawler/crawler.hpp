#pragma once

#include "bloom_filter.hpp"
#include "frontier.hpp"
#include "link_finder.hpp"
#include <hashblob.hpp>
#include <hashtable.hpp>
#include <pthread.h>
#include <stdexcept>
#include <sys/mman.h>
#include <csignal>

static constexpr int THREAD_COUNT = 20;
static constexpr int LINK_COUNT = 1000000; // ONE MILLION
static constexpr const char *BLOOM_FILE_PATH = "bloom_filter_dump.dat";
static constexpr const char *FRONTIER_PATH = "fronter_dump.dat";
static constexpr const char *SEED_LIST = "./seed_list.txt";
static constexpr size_t BLOOM_FILTER_NUM_OBJ = 1e6;
static constexpr double BLOOM_FILTER_FPR = 1e-4;
static constexpr size_t BLOB_THRESHOLD = 500'000;

class crawler {
public:
  crawler()
      : visited_urls(BLOOM_FILTER_NUM_OBJ, BLOOM_FILTER_FPR, BLOOM_FILE_PATH),
        crawl_frontier(FRONTIER_PATH, SEED_LIST),
        word_bank(new fast::hashtable) {}

  void run() {
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | 
                     OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);
    g_ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!g_ssl_ctx) {
      throw std::runtime_error("Failed to create SSL context");
    }
    // Set appropriate security level (if using OpenSSL 1.1.0+)
    SSL_CTX_set_security_level(g_ssl_ctx, 2);  // Reasonable security level

    // Set a broad cipher list that includes secure ciphers
    if (SSL_CTX_set_cipher_list(g_ssl_ctx, "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4") != 1) {
      SSL_CTX_free(g_ssl_ctx);
      throw std::runtime_error("Failed to set cipher list");
    }

    // Enable TLSv1.2 and TLSv1.3 if available
    SSL_CTX_set_min_proto_version(g_ssl_ctx, TLS1_2_VERSION);
    // Optional: set max version if needed
    SSL_CTX_set_max_proto_version(g_ssl_ctx, TLS1_3_VERSION);

    // For TLS 1.3 specific ciphers (if using OpenSSL 1.1.1+)
#if defined(TLS1_3_VERSION)
    SSL_CTX_set_ciphersuites(g_ssl_ctx, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256");
#endif

    for (auto &t : thread_pool) {
      // Lambda used as worker is not static
      if (pthread_create(
              &t, nullptr,
              [](void *arg) -> void * {
                auto self = static_cast<crawler *>(arg);
                self->worker(self->g_ssl_ctx);
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
    write_blob(itos(chunk_count) + ".txt", word_bank, bank_mtx);
    delete word_bank;
    SSL_CTX_free(g_ssl_ctx);
  }

  void shutdown() { shutdown_flag = 1; }

private:
  volatile sig_atomic_t shutdown_flag = 0;
  fast::crawler::bloom_filter<fast::string> visited_urls;
  fast::crawler::frontier crawl_frontier;
  fast::hashtable *word_bank;
  fast::mutex bank_mtx;
  SSL_CTX *g_ssl_ctx;
  int chunk_count{}; // TODO: search dir for next chunk count to continue crawling
  /*std::unordered_map<fast::string, std::unordered_set<fast::string>>*/
  pthread_t thread_pool[THREAD_COUNT]{};
  // We also need a map for robots.txt stuff

  static fast::string itos(int x) {
    fast::string s;
    while (x > 0) {
      int d = x % 10; // NOLINT
      x /= 10;        // NOLINT
      s.insert(0, static_cast<char>('0' + d));
    }
    return s;
  }

  void *worker(void *arg) {
    auto *ctx = static_cast<SSL_CTX*>(arg);
    
    link_finder html_scraper(ctx);
    while (!shutdown_flag) {
      fast::string url = "";
      // TODO: How do we get our start list to the right computers?
      while (url == "") {
        url = crawl_frontier.next();
      }
      visited_urls.insert(url);
      html_scraper.parse_url(url.begin());
      fast::vector<fast::string> extracted_links =
          html_scraper.parse_html(*word_bank, bank_mtx);
      for (auto &link : extracted_links) {
        if (!visited_urls.contains(link)) {
          crawl_frontier.insert(link);
        }
      }
      crawl_frontier.notify_crawled(url);
      if (word_bank->tokens() > BLOB_THRESHOLD) {
        write_blob(itos(chunk_count) + ".txt", word_bank, bank_mtx);
      }
    }
    std::cout << "Worker returning\n";
    return nullptr;
  }

  void write_blob(const fast::string &path, fast::hashtable *word_bank,
                  fast::mutex &bank_mtx) {
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

    auto blob = static_cast<fast::hashblob *>(mptr);
    fast::hashblob::write(*word_bank, blob);

    delete word_bank;
    word_bank = new fast::hashtable;

    munmap(mptr, space);
    close(fd);
    ++chunk_count;
    bank_mtx.unlock();
  }
};
