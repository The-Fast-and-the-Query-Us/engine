#pragma once

#include "condition_variable.hpp"
#include "flat_map.hpp"
#include "queue.hpp"
#include "scoped_lock.hpp"
#include "string.hpp"
#include "vector.hpp"

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

namespace fast::crawler {

class crawler;

static constexpr size_t MAX_LINKS = 500'000;

// TODO:
// - Make next probabilistic to go around our possibly incorrect heuristics
class frontier {
 public:
  frontier(const char* _save_path, const char* seed_list = nullptr)
      : save_path(_save_path == nullptr ? nullptr : strdup(_save_path)) {
    assert(save_path != nullptr);
    if (access(save_path, F_OK) == 0) {
      load();
      return;
    }
    priorities.resize(5);
    if (seed_list != nullptr) {
      load_seed_list(seed_list);
    }
  }

  ~frontier() {
    if (save_path != nullptr) {
      free(save_path);
    }
  }

  bool insert_no_mutex(fast::string& url) {
    int pri_level = calc_priority(url);
    if (pri_level < 0)
      return false;

    if (priorities[pri_level].size() < MAX_LINKS) {
      priorities[pri_level].push(url);
      ++num_links;
      cv.signal();

      return true;
    }

    return false;
  }

  bool insert(fast::string& url) {
    fast::scoped_lock lock(&mtx);
    return insert_no_mutex(url);
  }

  fast::string next(volatile sig_atomic_t* shutdown_flag = nullptr) {
    fast::scoped_lock lock(&mtx);

    while (num_links == 0 &&
           (shutdown_flag == nullptr || *shutdown_flag != 1)) {
      cv.wait(&mtx);
    }

    if (shutdown_flag != nullptr && *shutdown_flag == 1) {
      return "";
    }


    for (int64_t i = priorities.size() - 1; i >= 0; --i) {

      const auto flip = rand() % 100;
      if (flip >= 70) continue;

      fast::queue<fast::string>& curr_pri = priorities[i];

      if (!curr_pri.empty()) {
        const auto url = curr_pri.front();
        curr_pri.pop();

        return url;
      }
    }

    for (int64_t i = priorities.size() - 1; i >= 0; --i) {
      fast::queue<fast::string>& curr_pri = priorities[i];

      if (!curr_pri.empty()) {
        const auto url = curr_pri.front();
        curr_pri.pop();

        return url;
      }
    }


    return "";
  }

  void load_seed_list(const char* fp) {
    fast::scoped_lock lock(&mtx);
    int fd = open(fp, O_RDONLY);
    if (fd == -1) {
      perror("failed to open seedlist");
      return;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
      perror("fstat");
      close(fd);
      exit(EXIT_FAILURE);
    }
    size_t length = sb.st_size;

    char* file =
        static_cast<char*>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0));
    if (file == MAP_FAILED) {
      perror("mmap");
      return;
    }
    close(fd);

    char* end = file + length;
    while (file < end) {
      fast::string url{};
      while (file < end && *file != '\n') {
        url += *file++;
      }
      file++;

      insert_no_mutex(url);
    }
    // mtx.unlock();
    munmap(file, length);
    std::cout << "Succesfully loaded seed_list from " << fp << '\n';
  }

  void notify_crawled(fast::string& url) {
    fast::scoped_lock lock(&mtx);
    fast::string hostname = extract_hostname(url);
    //--crawl_cnt[hostname];
  }

  int save() {
    fast::scoped_lock lock(&mtx);

    int fd = open(save_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
      perror("Failed to open file in frontier save():");
      std::cerr << "save_path: " << save_path << '\n';
      return -1;
    }

    ssize_t num_links_written = write(fd, &num_links, sizeof(uint64_t));

    if (num_links_written == -1) {
      std::cerr << "Failed writing member num_links in frontier save()\n";
      close(fd);
      return -1;
    }

    size_t num_priorities = priorities.size();
    ssize_t num_priorities_written = write(fd, &num_priorities, sizeof(size_t));

    if (num_priorities_written == -1) {
      std::cerr << "Failed writing member num_priorities in frontier save()\n";
      close(fd);
      return -1;
    }

    int total_priority_bytes = 0;
    for (auto& level : priorities) {

      size_t pri_sz = level.size();
      ssize_t pri_sz_written = write(fd, &pri_sz, sizeof(size_t));

      if (pri_sz_written == -1) {
        std::cerr
            << "Failed writing size of priorities queue in frontier save()";
        close(fd);
        return -1;
      }

      total_priority_bytes += pri_sz_written;
      fast::queue<fast::string> t_q = level;

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

        ssize_t link_written = write(fd, f.c_str(), f_len);

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
    std::cout << "Succesfully saved frontier to " << save_path << '\n';
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

    size_t num_priorities{};
    ssize_t num_priorities_read = read(fd, &num_priorities, sizeof(size_t));

    if (num_priorities_read == -1) {
      std::cerr << "Failed writing member num_priorities in frontier save()\n";
      close(fd);
      return -1;
    }

    priorities.resize(num_priorities);

    for (auto& level : priorities) {
      size_t pri_sz{};
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
        level.push(l);
      }
    }

    close(fd);
    std::cout << "Succesfully loaded frontier from " << save_path << '\n';
    return tbr;
  }

  static fast::string extract_hostname(const fast::string& url) {
    fast::string hostname;
    size_t start_pos = 0;
    if (url.starts_with("http://")) {
      start_pos = 7;
    } else if (url.starts_with("https://")) {
      start_pos = 8;
    }

    for (size_t i = start_pos; i < url.size(); ++i) {
      if (url[i] == '/' || url[i] == '#') {
        break;
      }
      hostname += url[i];
    }

    if (hostname.starts_with("www.")) {
      hostname = hostname.substr(4, hostname.size() - 4);
    }

    return hostname;
  }

 private:
  friend class crawler;

  static constexpr uint8_t GOOD_LEN = 25;

  static constexpr uint8_t CRAWL_LIM = 3;

  static constexpr uint8_t TLD_LEN = 3;

  fast::mutex mtx;

  fast::condition_variable cv;

  uint64_t num_links{0};

  char* save_path;

  fast::vector<fast::queue<fast::string>> priorities;

  fast::flat_map<fast::string, uint8_t> crawl_cnt;

  // How to initialise this with fast::vector
  // TODO: INIT LIST FOR FAST VECTOR
  std::vector<fast::string> good_tld = {"com", "org", "gov", "net", "edu"};
  std::vector<fast::string> blacklist = {"signup", "signin", "login"};

  int8_t calc_priority(fast::string& url) {
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

    ssize_t idx = 0;

    // +1 point for secure protocol
    fast::string protocol;
    while (static_cast<size_t>(idx) < hostname.size() && hostname[idx] != ':')
      protocol += hostname[idx++];

    score += protocol == "https";

    // scan from back
    idx = hostname.size() - 1;

    // +1 point for tld
    fast::string tld;
    while (idx > 0 && hostname[idx] != '.')
      tld += hostname[idx--];

    if (tld.size() == 0) {
      return score;
    }
    tld.reverse(0, tld.size() - 1);

    for (auto d : good_tld) {
      if (tld == d) {
        ++score;
        break;
      }
    }

    size_t slash_cnt = 0;
    for (const auto c : url) {
      slash_cnt += c =='/';
    }

    // no protocol
    if (slash_cnt < 2) return -1;

    slash_cnt -= 2;

    if (slash_cnt > 7) return -1;

    if (slash_cnt < 2) ++score;

    return score;
  }
};

}  // namespace fast::crawler
