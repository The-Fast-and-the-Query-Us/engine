#pragma once

#include "../lib/condition_variable.hpp"
#include "../lib/queue.hpp"
#include "../lib/scoped_lock.hpp"
#include "../lib/string.hpp"
#include "../lib/vector.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <sys/fcntl.h>
#include <unistd.h>
#include <vector>

namespace fast {
namespace crawler {

// TODO:
//  - Add blacklist?
//  - How do we manage data and communication across all our latpops?
//  - redirects
class frontier {
public:
  frontier(const char *_save_path) : save_path(_save_path) {
    num_links = 0;
    priorities.resize(4);
  }

  void insert(fast::string &url) {
    fast::scoped_lock lock(&mtx);

    int pri_level = calc_priority(url);
    if (pri_level < 0)
      return;

    priorities[pri_level].push(url);

    ++num_links;
    cv.signal();
  }

  fast::string next() {
    fast::scoped_lock lock(&mtx);

    while (num_links == 0)
      cv.wait(&mtx);

    for (size_t i = priorities.size() - 1; i >= 0; --i) {
      fast::queue<fast::string> &curr_pri = priorities[i];

      if (!curr_pri.empty()) {

        const size_t n = curr_pri.size();
        for (size_t j = 0; j < n; ++j) {

          fast::string url = curr_pri.front();
          fast::string curr_hostname = extract_hostname(url);
          curr_pri.pop();

          if (crawl_cnt[curr_hostname] <= CRAWL_LIM) {
            ++crawl_cnt[curr_hostname];
            --num_links;
            return curr_hostname;
          }

          curr_pri.push(url);
        }
      }
    }

    return "";
  }

  void notify_crawled(fast::string &url) {
    fast::scoped_lock lock(&mtx);
    fast::string hostname = extract_hostname(url);
    --crawl_cnt[hostname];
  }

  int save() {
    fast::scoped_lock lock(&mtx);

    int fd = open(save_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
      perror("Failed to open file in frontier save()\n");
      return -1;
    }

    ssize_t num_links_written = write(fd, &num_links, sizeof(uint64_t));

    if (num_links_written == -1) {
      std::cerr << "Failed writing member num_links in frontier save()\n";
      close(fd);
      return -1;
    }

    int total_priority_bytes = 0;
    for (size_t i = 0; i < priorities.size(); ++i) {

      size_t pri_sz = priorities[i].size();
      ssize_t pri_sz_written = write(fd, &pri_sz, sizeof(size_t));

      if (pri_sz_written == -1) {
        std::cerr
            << "Failed writing size of priorities queue in frontier save()";
        close(fd);
        return -1;
      }

      total_priority_bytes += pri_sz_written;
      fast::queue<fast::string> t_q = priorities[i];

      for (size_t j = 0; j < pri_sz; ++j) {
        fast::string f = t_q.front();
        t_q.pop();
        size_t f_len = f.size();

        ssize_t link_len_written = write(fd, &f_len, sizeof(f_len));
        if (link_len_written == -1) {
          std::cerr << "Failed writing string length in frontier save(): "
                    << strerror(errno) << std::endl;
          close(fd);
          return -1;
        }
        total_priority_bytes += link_len_written;

        ssize_t link_written = write(fd, f.c_str(), f.size());

        if (link_written == -1) {
          std::cerr << "Failed writing an element of priority queue in "
                       "frontier save()";
          close(fd);
          return -1;
        }

        total_priority_bytes += link_written;
      }
    }
    close(fd);
    return num_links_written + total_priority_bytes;
  }

  int load() {
    fast::scoped_lock lock(&mtx);
    int tbr = 0;

    int fd = open(save_path, O_RDONLY);
    if (fd == -1) {
      std::cerr << "Failed to open file in frontier load()\n";
      return -1;
    }

    ssize_t num_links_read = read(fd, &num_links, sizeof(uint64_t));
    if (num_links_read == -1) {
      std::cerr << "failed reading num_links in frontier load()";
      close(fd);
      return -1;
    }
    tbr += num_links_read;

    for (size_t i = 0; i < priorities.size(); ++i) {
      size_t pri_sz;
      ssize_t num_pri_sz_read = read(fd, &pri_sz, sizeof(size_t));
      if (num_pri_sz_read == -1) {
        std::cerr << "failed reading pri_sz in frontier load()";
        close(fd);
        return -1;
      }
      tbr += num_pri_sz_read;

      for (size_t j = 0; j < pri_sz; ++j) {
        size_t l_len;
        ssize_t num_l_len_read = read(fd, &l_len, sizeof(size_t));
        if (num_l_len_read == -1) {
          std::cerr << "failed to read an elements size in frontier load()";
          close(fd);
          return -1;
        }
        tbr += num_l_len_read;

        fast::string l;
        l.resize(l_len);
        ssize_t num_l_read = read(fd, l.begin(), l_len);
        if (num_l_read == -1) {
          std::cerr << "failed to read an element in frontier load()";
          close(fd);
          return -1;
        }
        tbr += num_l_read;
        priorities[i].push(l);
      }
    }

    close(fd);
    return tbr;
  }

  static fast::string extract_hostname(fast::string &url) {
    fast::string hostname;
    uint8_t slash_cnt = 0;
    for (size_t i = 0; i < url.size(); ++i) {
      if (url[i] == '/' && ++slash_cnt == 3) {
        break;
      }
      hostname += url[i];
    }
    return hostname;
  }

private:
  static constexpr uint8_t GOOD_LEN = 25;

  static constexpr uint8_t CRAWL_LIM = 3;

  static constexpr uint8_t TLD_LEN = 3;

  fast::mutex mtx;

  fast::condition_variable cv;

  uint64_t num_links;

  const char *save_path;

  fast::vector<fast::queue<fast::string>> priorities;

  // Need to finish fast::hashmap
  std::map<fast::string, uint8_t> crawl_cnt;

  // How to initialise this with fast::vector
  std::vector<fast::string> good_tld = {"com", "org", "gov", "net", "edu"};
  std::vector<fast::string> blacklist = {"signup", "signin", "login"};

  int8_t calc_priority(fast::string &url) {
    fast::string hostname = extract_hostname(url);

    uint8_t score = 0;

    for (auto kywrd : blacklist) {
      if (kywrd.size() > url.size())
        continue;

      for (size_t i = 0; i <= (url.size() - kywrd.size()); ++i) {
        if (url[i] == kywrd[0] && url.substr(i, kywrd.size()) == kywrd) {
          return -1;
        }
      }
    }

    // +1 point for good length
    score += hostname.size() <= GOOD_LEN;

    size_t idx = 0;

    // +1 point for secure protocol
    fast::string protocol;
    while (idx < hostname.size() && hostname[idx] != ':')
      protocol += hostname[idx++];

    score += protocol == "https";

    // scan from back
    idx = hostname.size() - 1;

    // +1 point for tld
    fast::string tld;
    while (idx > 0 && hostname[idx] != '.')
      tld += hostname[idx--];

    tld.reverse(0, tld.size() - 1);

    for (auto d : good_tld) {
      if (tld == d) {
        ++score;
        break;
      }
    }

    return score;
  }
};

} // namespace crawler
} // namespace fast
