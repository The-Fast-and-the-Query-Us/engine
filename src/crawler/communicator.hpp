#pragma once

#include <fcntl.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstring>
//#include <stdexcept>
#include "html_file.hpp"

static const char *text_html_pattern = "content-type: text/html";
static const char *en_pattern = "lang=\"en\"";
static constexpr uint16_t MAX_PACKET_SIZE = 10240;
static constexpr uint16_t MAX_MESSAGE_SIZE = 8192;
static constexpr uint8_t REQUEST_TYPE_LEN = 16;
static constexpr uint8_t HOST_LEN = 6;
static constexpr uint8_t USER_AGENT_LEN = 38;
static constexpr uint8_t HTML_TAIL_LEN = 61;
static constexpr uint8_t TEXT_HTML_PATTERN_LEN = 23;
static constexpr uint8_t LANG_EN_PATTERN_LEN = 9;

namespace fast::crawler {

class communicator {
 private:
  struct addrinfo* address{};
  struct addrinfo hints{};
  int sock_fd = -1;
  SSL* ssl{};
  char *host{}, *port{}, *path{};

  void destroy_objects() {
    if (ssl) {
      SSL_free(ssl);
      ssl = nullptr;
    }
    if (sock_fd >= 0) {
      close(sock_fd);
      sock_fd = -1;
    }
    if (address) {
      freeaddrinfo(address);
      address = nullptr;
    }

    delete[] host;
    delete[] port;
    delete[] path;
    host = port = path = nullptr;
  }

 public:
  communicator(SSL_CTX* ctx, const char* host_, const char* port_, const char* path_)
    : host(new char[strlen(host_ ? host_ : "") + 1]),
      port(new char[strlen(port_ ? port_ : "") + 1]),
      path(new char[strlen(path_ ? path_ : "") + 1]) {

    strcpy(host, host_ ? host_ : "");
    strcpy(port, port_ ? port_ : "");
    strcpy(path, path_ ? path_ : "");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, "443", &hints, &address) < 0 || !address) {
      // throw std::runtime_error("getaddrinfo failed. Address not found.\n");
      destroy_objects();
      return;
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd < 0) {
      destroy_objects();
      // throw std::runtime_error("Failed to create socket.\n");
      return;
    }

    struct timeval timeout{};
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    // Get timeout
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
      perror("setsockopt failed");
      destroy_objects();
      return;
    }
    // Send timeout
    if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
      perror("setsockopt failed");
      destroy_objects();
      return;
    }

    if (!address || !address->ai_addr) {
      destroy_objects();
      return;
    }

    if (connect(sock_fd, address->ai_addr, address->ai_addrlen) < 0) {
      destroy_objects();
      perror("connect failed");
      // throw std::runtime_error("Connection to host failed.\n");
      return;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
      destroy_objects();
      perror("SSL_new failed.");
      // throw std::runtime_error("Failed to create SSL connection.\n");
      return;
    }

    if (SSL_set_fd(ssl, sock_fd) != 1) {
      destroy_objects();
      perror("SSL_set_fd failed.\n");
      return;
      // throw std::runtime_error("Failed to bind SSL to socket.\n");
    }

    // Ensure host is valid before using it
    if (!host || *host == '\0') {
      destroy_objects();
      return;
      // throw std::runtime_error("Invalid hostname for SSL.\n");
    }

    if (!SSL_set_tlsext_host_name(ssl, host)) {
      destroy_objects();
      return;
      // throw std::runtime_error("Failed to set SSL hostname: ");
    }

    int ssl_connect_result = SSL_connect(ssl);
    if (ssl_connect_result != 1) {
      int ssl_error = SSL_get_error(ssl, ssl_connect_result);
      char error_buffer[256]; // NOLINT
      snprintf(error_buffer, sizeof(error_buffer),
               "SSL_connect failed with error code: %d", ssl_error);

      destroy_objects();
      perror("SSL connection failed with error.\n");
      ERR_print_errors_fp(stderr);
      // throw std::runtime_error(error_buffer);
    }
  }

  ~communicator() { destroy_objects(); }

  void send_get_request() {
    if (!ssl)
      return;

    char get_request[MAX_MESSAGE_SIZE];
    get_request[0] = '\0';
    char* p = get_request;
    char* limit = get_request + MAX_MESSAGE_SIZE;
    const char* agent_email = "shivgov@umich.edu";

    if (p + REQUEST_TYPE_LEN + strlen(path) > limit) {
      // throw std::runtime_error("Get request buffer overflow.\n");
      destroy_objects();
      return;
    }
    strcat(p, "GET /");
    if (strlen(path)) {
      strcat(p, path);
    }
    strcat(p, " HTTP/1.1\r\n");

    if (p + HOST_LEN + strlen(host) + strlen(port) > limit) {
      // throw std::runtime_error("Get request buffer overflow.\n");
      destroy_objects();
      return;
    }
    strcat(p, "host: ");
    strcat(p, host);
    if (strlen(port)) {
      strcat(p, port);
    }
    strcat(p, "\r\n");

    if (p + USER_AGENT_LEN + strlen(agent_email) > limit) {
      // throw std::runtime_error("Get request buffer overflow.\n");
      destroy_objects();
      return;
    }
    strcat(p, "User-Agent: LinuxGetparser.2.0 ");
    strcat(p, agent_email);
    strcat(p, " (Linux)\r\n");

    if (p + HTML_TAIL_LEN > limit) {
      // throw std::runtime_error("Get request buffer overflow.\n");
      destroy_objects();
      return;
    }
    strcat(p,
           "Accept: */*\r\nAccept-Encoding: identity\r\nConnection: "
           "close\r\n\r\n");

    if (SSL_write(ssl, get_request, strlen(get_request)) <= 0) {
      destroy_objects();
      // throw std::runtime_error("SSL write failed.\n");
    }
  }

  void get_html(fast::crawler::html_file& html,
                const volatile sig_atomic_t* shutdown_flag) {
    if (!ssl || (shutdown_flag && *shutdown_flag)) {
      return;
    }

    html_file header;
    char buffer[MAX_PACKET_SIZE];
    ssize_t bytes{};
    const char* header_end = "\r\n\r\n";
    int header_match_pos = 0;
    bool found_header_end = false;
    bool invalid_html = false;
    uint8_t checked_for_en_twice = 0;

    while (true) {
      if (!ssl || invalid_html || (shutdown_flag && *shutdown_flag)) {
        break;
      }

      memset(buffer, 0, sizeof(buffer));

      bytes = SSL_read(ssl, buffer, sizeof(buffer));

      if (bytes <= 0) {
        int ssl_error = SSL_get_error(ssl, bytes);
        if (ssl_error == SSL_ERROR_WANT_READ ||
            ssl_error == SSL_ERROR_WANT_WRITE) {
          // Non-blocking operation, would block - try again
          if (shutdown_flag && *shutdown_flag)
            break;
          usleep(10000);
          break;
        }
        // Real error or connection closed
        break;
      }

      if (!found_header_end) {
        for (int i = 0; i < bytes; i++) {
          if (buffer[i] == header_end[header_match_pos]) {
            header_match_pos++;
            if (header_match_pos == 4) {  // Found \r\n\r\n
              found_header_end = true;
              if (header.size()) {
                header.add(buffer, i + 1);
              }
              if (!find_pattern_in_buffer(
                header.size() ? header.html : buffer,
                header.size() ? header.size() : bytes,
                text_html_pattern,
                TEXT_HTML_PATTERN_LEN
              )) {
                invalid_html = true;
                break;
              }
              if (i + 1 < bytes) {
                html.add(buffer + i + 1, bytes - i - 1);
              }
              break;
            }
          } else {
            header_match_pos = (buffer[i] == '\r') ? 1 : 0;
          }
        }
        if (!found_header_end)
          header.add(buffer, bytes);
      } else {
        html.add(buffer, bytes);
      }

      if (found_header_end && checked_for_en_twice < 2 && html.size()) {
        if (!find_pattern_in_buffer(html.html, html.size(), en_pattern, LANG_EN_PATTERN_LEN)) {
          break;
        }
        ++checked_for_en_twice;
      }
    }

    if (ssl) {
      SSL_shutdown(ssl);
      ssl = nullptr;
    }
  }

  static bool find_pattern_in_buffer(const char *buffer, size_t bytes, const char *pattern, const uint8_t PATTERN_LEN) {
    uint8_t pattern_index = 0;

    for (size_t i = 0; i < bytes; ++i) {
      if (lowercase(buffer[i]) == pattern[pattern_index]) {
        ++pattern_index;
        for (; pattern_index < PATTERN_LEN && i < bytes; ++pattern_index) {
          if (lowercase(buffer[i + pattern_index]) != pattern[pattern_index]) {
            i += pattern_index - 1;
            pattern_index = 0;
            break;
          }
        }
        if (pattern_index == PATTERN_LEN)
          return true;
      }
    }

    return false;
  }

  static char lowercase(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;  // NOLINT
  }
};

}  // namespace fast::crawler
