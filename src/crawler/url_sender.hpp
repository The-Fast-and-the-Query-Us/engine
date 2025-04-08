#pragma once

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
    const auto me = (const url_sender*) sender;
    while (1) {
      const auto client = accept(me->connect_fd, NULL, NULL);
      if (client < 0) return NULL;

      string buffer;
      
      if (recv_all(client, buffer)) {
        me->callback(buffer);
      }

      close(client);
    }
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

    pthread_create(&accept_thread, NULL, handle_connections, (void*) this);
  }

  /*
  * Hash url and send to the appropiate peer
  */
  void send_link(const string &url) {
    const auto &peer = ips[hash(url) % ips.size()];
    
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
      return;
    }

    if (!send_all(peer_fd, url)) {
      perror("send_link::fail to send to peer"); // this is recoverable
    } else {
      std::cout << "Exported link\n";
    }

    close(peer_fd);
  }

  ~url_sender() {
    shutdown(connect_fd, SHUT_RDWR);
    close(connect_fd);
    pthread_join(accept_thread, NULL);
  }
}; // url_sender

} // namespace
