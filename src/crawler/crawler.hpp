#pragma once

#include "bloom_filter.hpp"
#include "communicator.hpp"
#include "frontier.hpp"
#include "html_parser.hpp"
#include "lru_cache.hpp"
#include "robots.hpp"
#include "url_parser.hpp"
#include "url_sender.hpp"
#include "vector.hpp"
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <english.hpp>
#include <hashblob.hpp>
#include <hashtable.hpp>
#include <pthread.h>
#include <stdexcept>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <utility>

namespace fast::crawler {

static constexpr int THREAD_COUNT = 100;
static constexpr size_t BLOOM_FILTER_SIZE = 1e8;
static constexpr double BLOOM_FILTER_FPR = 1e-4;

static constexpr size_t ROBOTS_CACHE_ENTRIES = 4096;
static constexpr size_t BLOB_THRESHOLD = 30'000'000;
static constexpr const char* IP_PATH = "./ips.txt";

class crawler {
public:
  crawler()
      : visited_urls(
            access(get_bloomfilter_path().begin(), F_OK) == 0
                ? bloom_filter<fast::string>(get_bloomfilter_path().begin())
                : bloom_filter<fast::string>(BLOOM_FILTER_SIZE,
                                             BLOOM_FILTER_FPR,
                                             get_bloomfilter_path().begin())),
        robots_cache(ROBOTS_CACHE_ENTRIES),
        crawl_frontier(get_frontier_path().begin(), nullptr),
        word_bank(new fast::hashtable),
        log_fd(open(get_log_path().begin(), O_RDWR | O_CREAT | O_APPEND, 0777)),
        link_sender(IP_PATH,
                    std::bind(&crawler::add_url, this, std::placeholders::_1)) {
    const auto seed_path = get_seedlist_path();
    assert(log_fd >= 0);

    const auto fd = fopen(seed_path.c_str(), "r");
    if (fd != NULL) {
      std::cout << "Loading seedlist" << std::endl;
      char buffer[2000];
      fast::vector<Link> links;

      while (fgets(buffer, 2000, fd) != NULL) {
        buffer[strcspn(buffer, "\n\r")] = 0; // remove \r and \n
        auto url = fast::string(buffer);
        links.push_back(Link{url});
        // link_sender.push_back(Link(buffer));
      }

      link_sender.add_links(links);

      fclose(fd);
      // link_sender.flush(0);  // wont reach threshold for first sender

      std::cout << "Loaded seedlist" << std::endl;

    } else {
      std::cout << "No seedlist found" << std::endl;
    }
  }

  void run() {
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS |
                         OPENSSL_INIT_LOAD_CRYPTO_STRINGS,
                     nullptr);
    g_ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!g_ssl_ctx) {
      throw std::runtime_error("Failed to create SSL context");
    }
    // Set appropriate security level (if using OpenSSL 1.1.0+)
    SSL_CTX_set_security_level(g_ssl_ctx, 2); // Reasonable security level

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
        throw std::runtime_error("worker_thread creation failed\n");
      }
    }

    for (auto &t : thread_pool) {
      pthread_join(t, nullptr);
    }

    visited_urls.save();
    crawl_frontier.save();
    // No threads, so no need to lock
    if (word_bank->tokens() > 0) {
      write_blob();
    }
    std::cout << "Added " << doc_count << " docs to index\n";

    delete word_bank;
    SSL_CTX_free(g_ssl_ctx);
  }

  void shutdown() {
    shutdown_flag = 1;
    crawl_frontier.cv.broadcast();
  }

  static fast::string get_log_path() {
    fast::string path = getenv("HOME");
    path += "/.local/share/crawler/docs.log";
    return path;
  }

private:
  volatile sig_atomic_t shutdown_flag = 0;
  fast::crawler::bloom_filter<fast::string> visited_urls;
  fast::lru_cache<fast::string, fast::robots_filter> robots_cache;
  fast::crawler::frontier crawl_frontier;
  fast::hashtable *word_bank;

  fast::mutex cnt_mtx;
  fast::flat_map<fast::string, uint8_t> frontier_cnt;

  fast::mutex bank_mtx;
  int log_fd;
  SSL_CTX *g_ssl_ctx{};
  uint64_t doc_count{
      0}; // TODO: search dir for next chunk count to continue crawling
  /*std::unordered_map<fast::string, std::unordered_set<fast::string>>*/
  pthread_t thread_pool[THREAD_COUNT]{};
  // just for testing
  static const inline fast::string blacklist[] = {
      "login",       "signin",    "signup",       "account",  "password",
      "admin",       "profile",   "cart",         "checkout", "buy",
      "purchase",    "register",  "payment",      "shop",     "order",
      "cdn",         "static",    "assets",       "media",    "content",
      "cache",       "archive",   "porn",

      "fr",          "es",        "de",           "it",       "ru",
      "ja",          "zh",        "ko",           "ar",       "pt",
      "nl",          "pl",        "tr",           "bg",       "cs",
      "da",          "el",        "fi",           "he",       "hi",
      "hu",          "no",        "ro",           "sk",       "sl",
      "sv",          "th",        "vi",           "hr",       "ca",
      "et",          "fa",        "lt",           "lv",       "ms",
      "sr",          "sw",        "tl",           "ur",       "bn",
      "bs",          "cy",        "eo",           "eu",       "gl",
      "hy",          "is",        "ka",           "kk",       "km",
      "kn",          "ky",        "lo",           "mk",       "ml",
      "mn",          "mr",        "mt",           "my",       "ne",
      "si",          "sq",        "ta",           "te",       "uz",
      "zu",          "cn",        "jp",           "kr",       "ru",
      "tw",          "hk",        "br",           "mx",       "ar",
      "cl",          "pe",        "ve",           "za",       "ae",
      "sa",          "sg",        "in",           "pk",       "ph",
      "vn",          "th",        "my",           "il",       "tr",
      "ir",

      "baidu",       "weibo",     "qq",           "taobao",   "yandex",
      "vk",          "mail.ru",   "rambler",      "naver",    "daum",
      "yahoo.co.jp", "goo.ne.jp", "mercadolibre", "allegro",  "coccoc",
      "onet",        "interia",   "wp",           "sohu",     "sina",
      "alipay",      "tencent",   "bilibili",     "youku",    "tudou",
      "dmm",         "kakao",     "line",         "mixi",     "nicovideo",
      "pixiv",       "qzone",     "renren",       "wechat",   "weixin",
      "q",           "page",
  };

  url_sender link_sender;

  // static constexpr const char* BLOOM_FILE_PATH = getenv("HOME");
  // static constexpr const char* FRONTIER_PATH = "frontier_dump.dat";

  static fast::string get_bloomfilter_path() {
    fast::string path = getenv("HOME");
    path += "/.local/share/crawler/bloom_filter.bin";
    return path;
  }

  static fast::string get_frontier_path() {
    fast::string path = getenv("HOME");
    path += "/.local/share/crawler/frontier.bin";
    return path;
  }

  static fast::string get_seedlist_path() {
    fast::string path = getenv("HOME");
    path += "/.local/share/crawler/seed_list.txt";
    return path;
  }

  static fast::string u64tos(uint64_t x) {
    if (x == 0) {
      return "0";
    }
    fast::string res{};
    while (x > 0) {
      int d = x % 10;
      x /= 10;
      res.insert(0, '0' + d);
    }
    return res;
  }

  static fast::string get_blob_path() {
    fast::string path = getenv("HOME");
    path += "/.local/share/crawler/index/";
    fast::string chunk_count_path = getenv("HOME");
    chunk_count_path += "/.local/share/crawler/chunk_count.bin";
    if (access(chunk_count_path.begin(), F_OK) == 0) {
      int fd = open(chunk_count_path.begin(), O_RDONLY);
      assert(fd >= 0);
      uint64_t chunk_count{};
      assert(read(fd, &chunk_count, sizeof(uint64_t)) == sizeof(uint64_t));
      path += u64tos(chunk_count);
      close(fd);
    } else {
      int fd = open(chunk_count_path.begin(), O_RDWR | O_CREAT | O_TRUNC, 0777);
      assert(fd != 0);
      uint64_t chunk_count{0};
      std::cout << "creating chunk_count file, setting to " << chunk_count
                << '\n';
      assert(write(fd, &chunk_count, sizeof(uint64_t)) == sizeof(uint64_t));
      path += "0";
      close(fd);
    }
    return path;
  }

  static void increment_chunk_count() {
    fast::string chunk_count_path = getenv("HOME");
    chunk_count_path += "/.local/share/crawler/chunk_count.bin";
    if (access(chunk_count_path.begin(), F_OK) == 0) {
      int fd = open(chunk_count_path.begin(), O_RDWR);
      assert(fd != 0);
      uint64_t chunk_count{};
      assert(read(fd, &chunk_count, sizeof(uint64_t)) == sizeof(uint64_t));
      chunk_count++;
      lseek(fd, 0, SEEK_SET);
      std::cout << "updating chunk_count from " << chunk_count - 1 << " to "
                << chunk_count << '\n';
      assert(write(fd, &chunk_count, sizeof(uint64_t)) == sizeof(uint64_t));
      close(fd);
    } else {
      int fd = open(chunk_count_path.begin(), O_RDWR | O_CREAT | O_TRUNC, 0777);
      assert(fd != 0);
      uint64_t chunk_count{0};
      std::cout << "creating chunk_count file, setting to " << chunk_count
                << '\n';
      assert(write(fd, &chunk_count, sizeof(uint64_t)) == sizeof(uint64_t));
      close(fd);
    }
  }

  void *worker() {
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

      // We now set visited before adding to frontier!
      // if (!visited_urls.contains(url)) {
      //  visited_urls.insert(url);
      //} else {
      //  crawl_frontier.notify_crawled(url);
      //  continue;
      //}

      // if (url == "https://whereis.mit.edu/") {
      // crawl_frontier.notify_crawled(url);
      // continue;
      // }

      fast::crawler::url_parser url_parts(url.begin());
      if (!url_parts.host || !*url_parts.host || !url_parts.port ||
          !url_parts.path || !isalnum(*url_parts.host)) {
        crawl_frontier.notify_crawled(url);
        continue;
      }

      const auto domain = url_parser::get_base_root(url);

      cnt_mtx.lock();
      frontier_cnt[fast::english::strip_url_prefix(domain)]--;
      cnt_mtx.unlock();

      SSL_CTX *ctx_cpy = g_ssl_ctx;

      if (!ctx_cpy)
        continue;

      fast::crawler::communicator ssl_connection(
          ctx_cpy, url_parts.host, url_parts.port, url_parts.path);

      ssl_connection.send_get_request();
      fast::crawler::html_file html;
      ssl_connection.get_html(html, &shutdown_flag);

      if (!html.size()) {
        crawl_frontier.notify_crawled(url);
        continue;
      }

      html_parser parser(html.html, html.size());

      bank_mtx.lock();

      for (fast::string &word : parser.words) {
        english::normalize_text(word);

        if (word.size() > 0) {
          word_bank->add(word);
        }
      }

      for (fast::string &word : parser.titleWords) {
        english::normalize_text(word);

        if (word.size() > 0) {
          word += '#';
          word_bank->add(word);
        }
      }

      if (++doc_count % 4095 == 0) {
        struct timespec current_time;
        long current_time_ms;

        clock_gettime(CLOCK_REALTIME, &current_time);

        current_time_ms =
            (current_time.tv_sec * 1000) + (current_time.tv_nsec / 1000000);
        assert(dprintf(log_fd, "%lu\n",
                       static_cast<unsigned long>(current_time_ms)) >= 0);
      };
      word_bank->add_doc(url);

      if (word_bank->tokens() > BLOB_THRESHOLD) {
        visited_urls.save();
        crawl_frontier.save();
        write_blob();
      }

      bank_mtx.unlock();

      for (auto &link : parser.links) {
        if (link.URL.size() == 0 || link.URL[0] == '#' ||
            is_blacklisted(link.URL) || link.URL.size() > 400) {
          link.URL = "";
          continue;
        }

        link.URL = url_parser::url_join(url, link.URL);

        if (link.URL.size() > 0 && link.URL.back() == '/') link.URL.pop_back();
      }

      link_sender.add_links(parser.links);

      crawl_frontier.notify_crawled(url);
    }

    std::cout << "Worker " << pthread_self() << " returning\n";
    return nullptr;
  }

  // call back function for recving urls to crawl

  void add_url(string& url) {
    static constexpr uint8_t MAX_CNT = 5;
    static constexpr uint8_t WL_MAX_CNT = 40;

    const auto domain = url_parser::get_base_root(url);
    const auto dom_no_prot = fast::english::strip_url_prefix(domain);

    fast::string stripped = fast::english::strip_url_prefix(url);

    bool whitelisted_dom = dom_no_prot == "nytimes.com"
      || dom_no_prot == "en.wikipedia.org" 
      || dom_no_prot == "stackoverflow.com"
      || dom_no_prot == "bbc.com"
      || dom_no_prot == "britannica.com"
    ;

    cnt_mtx.lock();

    bool in_robots_cache = robots_cache.contains(stripped);

    if (!in_robots_cache) {
      // unlock mtx before fetching new robots.txt because it could take a while
      cnt_mtx.unlock();
      robots_filter rbts(stripped);
      auto cpy = stripped;
      cnt_mtx.lock();
      robots_cache.insert(std::move(cpy), std::move(rbts));
    }

    auto &rbts_fltr = robots_cache[stripped];

    if (frontier_cnt[dom_no_prot] < (whitelisted_dom ? WL_MAX_CNT : MAX_CNT) &&
        !visited_urls.contains(stripped) && rbts_fltr.allow(stripped)) {
      if (crawl_frontier.insert(url)) {
        visited_urls.insert(stripped);

        frontier_cnt[dom_no_prot]++;
      }
    }

    cnt_mtx.unlock();
  }

  void write_blob() {
    fast::string path = get_blob_path();
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

    auto blob = static_cast<fast::hashblob *>(mptr);
    fast::hashblob::write(*word_bank, blob);

    delete word_bank;
    word_bank = new fast::hashtable;

    munmap(mptr, space);
    close(fd);
    std::cout << "Successfully wrote blob to " << path.begin() << '\n';
    increment_chunk_count();
  }

public:
  static bool is_blacklisted(const fast::string &url) {
    if (url.size() == 0)
      return true;

    if (url.ends_with("print.html") ||
        url.view().trim_prefix(1).contains("http"))
      return true;

    if (url.view().contains("wikipedia") &&
        !url.starts_with("https://en.wikipedia"))
      return true;

    const char* word_start = nullptr;
    const char* url_end = url.begin() + url.size();

    for (const char *p = url.begin(); p <= url_end; ++p) {
      bool is_delimiter = (p == url_end) || (*p == '/') || (*p == '.') ||
                          (*p == '#') || (*p == '?') || (*p == '&') ||
                          (*p == '=') || (*p == '-') || (*p == '_') ||
                          (*p == ':');

      if (is_delimiter) {
        if (word_start && p > word_start) {
          size_t word_len = p - word_start;

          if (word_len > 1) {
            for (const auto &banned : blacklist) {
              if (banned == fast::string_view(word_start, word_len)) {
                return true;
              }
            }
          }
        }
        word_start = nullptr;
      } else if (!word_start) {
        word_start = p;
      }
    }

    return false;
  }

  static bool is_alphabet(char c) { return isalnum(c); }

  static void lower(fast::string &word) {
    for (char &c : word) {
      c = tolower(c);
    }
  }
};
} // namespace fast::crawler
