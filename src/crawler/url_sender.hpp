#pragma once

#include "condition_variable.hpp"
#include "frontier.hpp"
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

namespace fast::crawler {


template<size_t MAX_BUFFER = 100>
class url_sender {
  int connect_fd;
  vector<string> ips;
  vector<vector<string>> send_buffers;

  pthread_t accept_thread;
  pthread_t send_thread;

  fast::condition_variable cv;
  fast::mutex mtx;
  volatile bool die = false;

  const std::function<void(string&)> callback;

  static constexpr unsigned PORT = 8090;

  /*
   * Call this->callback whenever a string is recieved
   */
  static void *handle_connections(void *sender) {
    const auto me = (const url_sender*) sender;
    while (1) {
      const auto client = accept(me->connect_fd, NULL, NULL);
      if (client < 0) return NULL;

      uint32_t count;
      if (recv_all(client, count)) {
        string buffer;

        while (count--) {
          if (recv_all(client, buffer)) {
            me->callback(buffer);
          } else {
            perror("url_sender::handle_conn : Fail to recv link");
            break;
          }
        }
      } else {
        perror("url_sender::handle_conn : Fail to get count");
      }
      
      close(client);
    }
  }

  static void *handle_send(void *sender) {
    const auto me = (url_sender*) sender;
    me->mtx.lock();

    while (1) {
      while (!me->die && me->get_snd_idx() == -1) me->cv.wait(&me->mtx);
      if (me->die) {
        me->mtx.unlock();
        return NULL;
      }

      const auto peer_idx = me->get_snd_idx();
      me->mtx.unlock();
      const auto &peer = me->ips[peer_idx];
      
      const auto peer_fd = socket(AF_INET, SOCK_STREAM, 0);
      if (peer_fd < 0) {
        perror("send_link::socket()");
        exit(1); // maybe dont exit?
      }

      struct sockaddr_in addr{};
      addr.sin_family = AF_INET;
      addr.sin_port = htons(PORT);

      if (inet_pton(AF_INET, peer.c_str(), &addr.sin_addr) <= 0) {
        perror("send_link::inet_pton");
        close(peer_fd);
        exit(1);
      }

      if (connect(peer_fd, (sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("send_link::connect");
        close(peer_fd);
        me->mtx.lock();
        continue;
      }

      me->mtx.lock();

      uint32_t count = me->send_buffers[peer_idx].size();
      if (send_all(peer_fd, count)) {
        for (const auto &link : me->send_buffers[peer_idx]) {
          if (!send_all(peer_fd, link)) {
            perror("URL_SENDER:: Fail to send link");
            break;
          }
        }
      } else {
        perror("URL_SENDER:: Fail to send count");
      }

      me->send_buffers[peer_idx].clear();
      close(peer_fd);
    }
  }

  // get first idx to send to
  ssize_t get_snd_idx() const {
    for (size_t i = 0; i < send_buffers.size(); ++i) {
      if (send_buffers[i].size() > MAX_BUFFER) {
        return i;
      }
    }
    return -1;
  }

public:
  url_sender(const fast::string &ip_path, std::function<void(string&)> callback) : callback(callback) {
    connect_fd = socket(AF_INET, SOCK_STREAM, 0);

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

    if (listen(connect_fd, SOMAXCONN) < 0) {
      close(connect_fd);
      perror("url_serner::listen");
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

    send_buffers.resize(ips.size());

    pthread_create(&accept_thread, NULL, handle_connections, (void*) this);
#if defined(__linux__)
    cpu_set_t cpuset;
    // pin accept thread to CPU 0
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(accept_thread, sizeof(cpu_set_t), &cpuset);
#endif

    pthread_create(&send_thread, NULL, handle_send, (void*) this);
  }

  void add_links(const fast::vector<Link> &links) {
    mtx.lock();

    for (const auto &link : links) {
      if (link.URL.size() == 0) continue;
      
      // get hash
    }

    for (const auto &b : send_buffers) {
      if (b.size() > MAX_BUFFER) {
        cv.signal();
        break;
      }
    }

    mtx.unlock();
  }

  ~url_sender() {
    shutdown(connect_fd, SHUT_RDWR);
    close(connect_fd);
    pthread_join(accept_thread, NULL);

    die = true;
    cv.signal();
    pthread_join(send_thread, NULL);
  }
}; // url_sender

} // namespace
