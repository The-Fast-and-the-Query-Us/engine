#pragma once

#include "condition_variable.hpp"
#include "frontier.hpp"
#include "hash.hpp"
#include "html_parser.hpp"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string.hpp>
#include <sys/socket.h>
#include <vector.hpp>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <network.hpp>
#include <mutex.hpp>
#include <english.hpp>

namespace fast::crawler {

class url_sender {

  int connect_fd;
  vector<string> ips;
  pthread_t accept_thread;

  const std::function<void(string&)> callback;

  static constexpr unsigned PORT = 8090;

  /*
   * Call this->callback whenever a string is recieved
   */
  static void *handle_connections(void *sender) {
    char buffer[4096];
    const auto me = (const url_sender*) sender;
    while (1) {
      const auto len = recvfrom(me->connect_fd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);

      if (len < 0) {
        perror("url_accept returning");
        return NULL;
      }

      ssize_t i = 0;
      while (i < len) {
        string link(buffer + i);
        i += link.size() + 1;
        me->callback(link);
      }
    }
  }


public:
  url_sender(const fast::string &ip_path, std::function<void(string&)> callback) : callback(callback) {
    connect_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (connect_fd < 0) {
      perror("url_sender::socket()");
      exit(1);
    }

    int yes = 1;
    if (setsockopt(connect_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
      perror("url_sender::setsockopt()");
      close(connect_fd);
      exit(1);
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(connect_fd, (sockaddr*) &addr, sizeof(addr)) < 0) {
      close(connect_fd);
      perror("url_sender::bind()");
      exit(1);
    }

    auto file = fopen(ip_path.c_str(), "r");

    if (file == NULL) {
      close(connect_fd);
      perror("url_sender::fopen");
      exit(1);
    }

    char IP_BUFFER[20];
    while (fgets(IP_BUFFER, 20, file) != NULL) {
      IP_BUFFER[strcspn(IP_BUFFER, "\r\n")] = 0; // remove trailing \r or \n
      ips.emplace_back(IP_BUFFER);
    }

    fclose(file);

    if (ips.size() == 0) {
      close(connect_fd);
      perror("Must have at least 1 IP");
      exit(1);
    }

    pthread_attr_t attr;
    if (pthread_attr_init(&attr) == -1) {
      std::cout << "attr init" << std::endl;
      exit(1);
    }

    if (geteuid() == 0) {
      sched_param param{};
      param.sched_priority = 1;

      pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
      pthread_attr_setschedparam(&attr, &param);

      pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    }

    if (pthread_create(&accept_thread, &attr, handle_connections, (void*) this) != 0) {
      perror("FAIL TO CREATE");
      exit(1);
    }

  }

  void add_links(const fast::vector<Link> &links) {
    constexpr size_t PACKET_SZ = 1400;

    fast::vector<fast::pair<char [PACKET_SZ], size_t>> bufs(ips.size());

    const auto fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
      perror("add_links");
      return;
    }

    for (const auto &link : links) {
      if (link.URL.size() == 0) continue;
      if (link.URL.size() > 400) continue;
      
      const auto trimmed = english::strip_url_prefix(link.URL);
      fast::string lower(trimmed);

      for (auto &c : lower) c = tolower(c);

      const auto hv = hash(lower) % ips.size();

      if (bufs[hv].second + link.URL.size() + 1 >= PACKET_SZ) {

        struct sockaddr_in dest{};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(PORT);
        inet_pton(AF_INET, ips[hv].c_str(), &dest.sin_addr);

        sendto(fd, bufs[hv].first, bufs[hv].second, 0, (sockaddr *) &dest, sizeof(dest));

        bufs[hv].second = 0;
      }

      memcpy(bufs[hv].first + bufs[hv].second, link.URL.c_str(), link.URL.size() + 1);
      bufs[hv].second += link.URL.size() + 1;
    }

    for (size_t i = 0; i < bufs.size(); ++i) {
      if (bufs[i].second == 0) continue;

        struct sockaddr_in dest{};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(PORT);
        inet_pton(AF_INET, ips[i].c_str(), &dest.sin_addr);

        sendto(fd, bufs[i].first, bufs[i].second, 0, (sockaddr *) &dest, sizeof(dest));
    }

    close(fd);
  }

  ~url_sender() {
    shutdown(connect_fd, SHUT_RDWR);
    close(connect_fd);
    pthread_join(accept_thread, NULL);
  }
}; // url_sender

} // namespace
