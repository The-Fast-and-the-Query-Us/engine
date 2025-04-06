#pragma once

#include <pthread.h>
#include <sys/mman.h>
#include <csignal>
#include <hashblob.hpp>
#include <hashtable.hpp>
#include <stdexcept>
#include "bloom_filter.hpp"
#include "communicator.hpp"
#include "frontier.hpp"
#include "html_parser.hpp"
#include "url_parser.hpp"

static constexpr int THREAD_COUNT = 5;
static constexpr int LINK_COUNT = 1000000;  // ONE MILLION
static constexpr const char* BLOOM_FILE_PATH = "bloom_filter_dump.dat";
static constexpr const char* FRONTIER_PATH = "frontier_dump.dat";
static constexpr const char* SEED_LIST = "./seed_list.txt";
static constexpr size_t BLOOM_FILTER_NUM_OBJ = 1e8;
static constexpr double BLOOM_FILTER_FPR = 1e-4;
static constexpr size_t BLOB_THRESHOLD = 500'000;

namespace fast::crawler {
class crawler {
 public:
  crawler()
      : visited_urls(BLOOM_FILTER_NUM_OBJ, BLOOM_FILTER_FPR, BLOOM_FILE_PATH),
        crawl_frontier(FRONTIER_PATH, SEED_LIST),
        word_bank(new fast::hashtable) {}

  void run() {
    OPENSSL_init_ssl(
        OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS,
        nullptr);
    g_ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!g_ssl_ctx) {
      throw std::runtime_error("Failed to create SSL context");
    }
    // Set appropriate security level (if using OpenSSL 1.1.0+)
    SSL_CTX_set_security_level(g_ssl_ctx, 2);  // Reasonable security level

    // Set a broad cipher list that includes secure ciphers
    if (SSL_CTX_set_cipher_list(g_ssl_ctx,
                                "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4") != 1) {
      SSL_CTX_free(g_ssl_ctx);
      throw std::runtime_error("Failed to set cipher list");
    }

    // Enable TLSv1.2 and TLSv1.3 if available
    SSL_CTX_set_min_proto_version(g_ssl_ctx, TLS1_2_VERSION);
    // Optional: set max version if needed
    SSL_CTX_set_max_proto_version(g_ssl_ctx, TLS1_3_VERSION);

    // For TLS 1.3 specific ciphers (if using OpenSSL 1.1.1+)
#if defined(TLS1_3_VERSION)
    SSL_CTX_set_ciphersuites(g_ssl_ctx,
                             "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_"
                             "SHA256:TLS_AES_128_GCM_SHA256");
#endif

    for (auto& t : thread_pool) {
      // Lambda used as worker is not static
      if (pthread_create(
              &t, nullptr,
              [](void* arg) -> void* {
                auto self = static_cast<crawler*>(arg);
                self->worker();
                return nullptr;
              },
              this)) {
        throw std::runtime_error("worker_thread creation failed\n");
      }
    }

    for (auto& t : thread_pool) {
      pthread_join(t, nullptr);
    }

    visited_urls.save();
    crawl_frontier.save();
    // No threads, so no need to lock
    if (word_bank->tokens() > 0) {
      write_blob(get_blob_path(chunk_count));
    }

    delete word_bank;
    SSL_CTX_free(g_ssl_ctx);
  }

  void shutdown() {
    shutdown_flag = 1;
    crawl_frontier.cv.broadcast();
  }

 private:
  volatile sig_atomic_t shutdown_flag = 0;
  fast::crawler::bloom_filter<fast::string> visited_urls;
  fast::crawler::frontier crawl_frontier;
  fast::hashtable* word_bank;
  fast::mutex bank_mtx;
  fast::mutex ssl_mtx;
  SSL_CTX* g_ssl_ctx{};
  int chunk_count{};  // TODO: search dir for next chunk count to continue crawling
  /*std::unordered_map<fast::string, std::unordered_set<fast::string>>*/
  pthread_t thread_pool[THREAD_COUNT]{};
  // We also need a map for robots.txt stuff

  static fast::string get_blob_path(int chunk_count) {
    if (chunk_count == 0) {
      return "index.dat/0";
    }
    fast::string num_str{};
    while (chunk_count > 0) {
      int d = chunk_count % 10;  // NOLINT
      chunk_count /= 10;         // NOLINT
      num_str.insert(0, static_cast<char>('0' + d));
    }
    std::cout << "num_str: " << num_str.begin() << '\n';

    fast::string path_str = "index.dat/";
    path_str += num_str;
    return path_str;
  }

  void* worker() {
    // Get url from frontier
    // parse url
    // Setup connection for this url
    // setup get request - can this be a one time thing?
    // send get request
    // Get html and store it in a html_file obj
    // Parse the html and get words, titleWords, and links
    // Add links to frontier
    // Add words, titleWords, and end_doc url -- lock for this in one go
    // If the word_bank is big enough, write this out to a chunk
    // If we have not shutdown, then repeat
    //
    // Separate:
    //  -
    while (!shutdown_flag) {
      fast::string url = "";
      // TODO: How do we get our start list to the right computers?
      int attempts = 0;
      while (url == "" && attempts < 3 && !shutdown_flag) {
        url = crawl_frontier.next(&shutdown_flag);
        if (url == "") {
          usleep(100'000);
          ++attempts;
        }
      }

      if (url == "" || shutdown_flag) {
        continue;
      }

      visited_urls.insert(url);

      fast::crawler::url_parser url_parts(url.begin());
      if (!url_parts.host || !*url_parts.host) {
        crawl_frontier.notify_crawled(url);
        continue;
      }

      ssl_mtx.lock();
      SSL_CTX* ctx_cpy = g_ssl_ctx;
      ssl_mtx.unlock();

      if (!ctx_cpy)
        continue;

      fast::crawler::communicator ssl_connection(
          ctx_cpy, url_parts.host, url_parts.port, url_parts.path);

      ssl_connection.send_get_request();
      fast::crawler::html_file html;
      ssl_connection.get_html(html);

      if (!html.size()) {
        crawl_frontier.notify_crawled(url);
        continue;
      }

      html_parser parser(html.html, html.size());

      bank_mtx.lock();

      for (fast::string& word : parser.words) {
        if (word.size() == 0) {
          continue;
        }
        while (!is_alphabet(word[0])) {
          if (word.size() == 0) {
            break;
          }
          word = word.substr(1, word.size() - 1);
        }
        if (word.size() == 0) {
          continue;
        }
        while (!is_alphabet(word[word.size() - 1])) {
          if (word.size() == 0) {
            break;
          }
          word = word.substr(0, word.size() - 1);
        }
        if (word.size() == 0) {
          continue;
        }
        lower(word);
        word_bank->add(word);
      }

      for (fast::string& word : parser.titleWords) {
        if (word.size() == 0) {
          continue;
        }
        while (!is_alphabet(word[0])) {
          if (word.size() == 0) {
            break;
          }
          word = word.substr(1, word.size() - 1);
        }
        while (!is_alphabet(word[word.size() - 1])) {
          if (word.size() == 0) {
            break;
          }
          word = word.substr(0, word.size() - 1);
        }
        if (word.size() == 0) {
          continue;
        }
        lower(word);
        word += '#';
        word_bank->add(word);
      }

      word_bank->add_doc(url);

      if (word_bank->tokens() > BLOB_THRESHOLD) {
        write_blob(get_blob_path(chunk_count));
      }

      bank_mtx.unlock();

      for (auto& link : parser.links) {
        if (!visited_urls.contains(link.URL)) {
          crawl_frontier.insert(link.URL);
        }
      }

      crawl_frontier.notify_crawled(url);
    }

    std::cout << "Worker returning\n";
    return nullptr;
  }

  void write_blob(const fast::string& path) {
    std::cout << "writing blob " << path.begin() << '\n';
    auto fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);

    if (fd == -1) {
      perror("open failed");
      return;
    }

    size_t space = fast::hashblob::size_needed(*word_bank);

    if (ftruncate(fd, space) == -1) {
      perror("Ftruncate fail");
      close(fd);
      return;
    }

    auto mptr = mmap(nullptr, space, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mptr == MAP_FAILED) {
      perror("cant mmap");
      close(fd);
      return;
    }

    auto blob = static_cast<fast::hashblob*>(mptr);
    fast::hashblob::write(*word_bank, blob);

    delete word_bank;
    word_bank = new fast::hashtable;

    munmap(mptr, space);
    close(fd);
    ++chunk_count;
  }

  static bool is_alphabet(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  static void lower(fast::string& word) {
    for (char& c : word) {
      if (is_alphabet(c) && c <= 'Z' && c >= 'A') {
        c = c + 32;  // NOLINT
      }
    }
  }
};
}  // namespace fast::crawler
