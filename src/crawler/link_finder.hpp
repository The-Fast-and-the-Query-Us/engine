#pragma once

#include "hashtable.hpp"
#include "html_file.hpp"
#include "html_parser.hpp"
#include "mutex.hpp"
#include "string.hpp"
#include "vector.hpp"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static constexpr uint16_t MAX_PACKET_SIZE = 10240;
static constexpr uint16_t MAX_MESSAGE_SIZE = 8192;
static constexpr uint8_t REQUEST_TYPE_LEN = 16;
static constexpr uint8_t HOST_LEN = 6;
static constexpr uint8_t USER_AGENT_LEN = 38;
static constexpr uint8_t HTML_TAIL_LEN = 61;
static constexpr char COLON = ':';
static constexpr char SLASH = '/';

class link_finder {
public:
  link_finder(SSL_CTX *c) : ctx(c) {}

  void parse_url(const char *file) {
    url = file;
    path_buffer = new char[url.size() + 1];

    const char *f{};
    char *t{};
    for (t = path_buffer, f = url.begin(); (*t++ = *f++);) {
    }
    service = path_buffer;
    char *p{};
    for (p = path_buffer; *p && *p != COLON; p++) {
    }
    if (*p) {
      // Mark the end of the Service.
      *p++ = 0;
      if (*p == SLASH) 
        ++p;
      if (*p == SLASH)
        ++p;

      host = p;
      for (; *p && *p != SLASH && *p != COLON; p++) {}

      if (*p == COLON) {
        *p++ = 0;
        port = +p;
        for (; *p && *p != SLASH; p++) {}
      } else {
        port = p;
      }

      if (*p) {
        *p++ = 0;
      }

      path = p;
    } else {
      host = path = p;
    }
  }

  ~link_finder() { destroy_objects(); }

  fast::vector<fast::string> parse_html(fast::hashtable &word_bank,
                                        fast::mutex &bank_mtx) {
    fast::crawler::html_file html{};
    get_html(html);
    if (!html.size())
      return {};

    HtmlParser parser(html.html, html.size());
    fast::vector<fast::string> links(parser.links.size());
    for (size_t i = 0; i < links.size(); ++i) {
      links[i] = parser.links[i].URL;
    }

    bank_mtx.lock();
    for (fast::string &word : parser.words) {
      if (word.size() == 0) {
        continue;
      }
      while (!is_alphabet(word[0]))
        word = word.substr(1, word.size() - 1);
      while (!is_alphabet(word[word.size() - 1]))
        word = word.substr(0, word.size() - 1);
      lower(word);
      word_bank.add(word);
    }
    for (fast::string &word : parser.titleWords) {
      if (word.size() == 0)
        continue;
      while (!is_alphabet(word[0]))
        word = word.substr(1, word.size() - 1);
      while (!is_alphabet(word[word.size() - 1]))
        word = word.substr(0, word.size() - 1);
      lower(word);
      word += '#';
      word_bank.add(word);
    }
    word_bank.add_doc(url);
    bank_mtx.unlock();

    destroy_objects();

    return links;
  }

private:
  struct addrinfo *address{};
  struct addrinfo hints{};
  int sock_fd = -1;
  fast::string url;
  SSL_CTX *ctx{};
  SSL *ssl{};
  char *path_buffer{}, *service{}, *host{}, *port{}, *path{};

  void setup_connection() {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(host, "443", &hints, &address) < 0) {
      throw std::runtime_error("getaddrinfo failed. Address not found.\n");
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connect(sock_fd, address->ai_addr, address->ai_addrlen) < 0) {
      perror("brev");
      throw std::runtime_error("Connection to host failed.\n");
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
      close(sock_fd);
      perror("SSL_new failed.");
      throw std::runtime_error("Failed to create SSL connection.\n");
    }

    if (SSL_set_fd(ssl, sock_fd) != 1) {
      SSL_free(ssl);
      close(sock_fd);
      perror("SSL_set_fd failed.\n");
      throw std::runtime_error("Failed to bind SSL to socket.\n");
    }
    
    if (!SSL_set_tlsext_host_name(ssl, host)) {
      SSL_free(ssl);
      close(sock_fd);
      throw std::runtime_error("Failed to set SSL hostname: ");
    }

    if (SSL_connect(ssl) != 1) {
      SSL_free(ssl);
      close(sock_fd);
      perror("SSL connection failed with error.\n");
      ERR_print_errors_fp(stderr);
      throw std::runtime_error("SSL_connect returned -1");
    }
  }

  void send_get_request() {
    char get_request[MAX_MESSAGE_SIZE];
    get_request[0] = '\0';
    char *p = get_request;
    char *limit = get_request + MAX_MESSAGE_SIZE;
    const char *agent_email = "shivgov@umich.edu";

    if (p + REQUEST_TYPE_LEN + strlen(path) > limit) {
      throw std::runtime_error("Get request buffer overflow.\n");
    }
    strcat(p, "GET /");
    if (strlen(path)) {
      strcat(p, path);
    }
    strcat(p, " HTTP/1.1\r\n");

    if (p + HOST_LEN + strlen(host) + strlen(port) > limit) {
      throw std::runtime_error("Get request buffer overflow.\n");
    }
    strcat(p, "host: ");
    strcat(p, host);
    if (strlen(port)) {
      strcat(p, port);
    }
    strcat(p, "\r\n");

    if (p + USER_AGENT_LEN + strlen(agent_email) > limit) {
      throw std::runtime_error("Get request buffer overflow.\n");
    }
    strcat(p, "User-Agent: LinuxGetparser.2.0 ");
    strcat(p, agent_email);
    strcat(p, " (Linux)\r\n");

    if (p + HTML_TAIL_LEN > limit) {
      throw std::runtime_error("Get request buffer overflow.\n");
    }
    strcat(p, "Accept: */*\r\nAccept-Encoding: identity\r\nConnection: "
              "close\r\n\r\n");

    /*for (int i = 0; i < strlen(get_request); ++i) {*/
    /*  std::cout << get_request[i];*/
    /*}*/

    if (SSL_write(ssl, get_request, strlen(get_request)) <= 0) {
      SSL_free(ssl);
      SSL_CTX_free(ctx);
      close(sock_fd);
      throw std::runtime_error("SSL write failed.\n");
    }
  }

  void get_html(fast::crawler::html_file &html) {
    char buffer[MAX_PACKET_SIZE];
    ssize_t bytes{};
    bool found_header_end = false;
    const char *header_end = "\r\n\r\n";
    int header_match_pos = 0;

    setup_connection();
    send_get_request();

    while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
      if (!found_header_end) {
        for (int i = 0; i < bytes; i++) {
          if (buffer[i] == header_end[header_match_pos]) {
            header_match_pos++;
            if (header_match_pos == 4) { // Found \r\n\r\n
              found_header_end = true;
              html.add(buffer + i + 1, bytes - i - 1);
              // write(1, buffer + i + 1, bytes - (i + 1));
              break;
            }
          } else {
            header_match_pos = (buffer[i] == '\r') ? 1 : 0;
          }
        }
      } else {
        html.add(buffer, bytes);
      }
    }
    if (bytes < 0) {
      return;
    }

    if (SSL_shutdown(ssl) < 0) {
      destroy_objects();
      throw std::runtime_error("SSL shutdown failed with error.\n");
    }
  }

  static bool is_alphabet(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  static void lower(fast::string &word) {
    for (char &c : word) {
      if (is_alphabet(c) && c <= 'Z' && c >= 'A') {
        c = c + 32;
      }
    }
  }

  // TODO: strip punctuation, read header to better prune shit
  void destroy_objects() {
    if (path_buffer) {
      delete[] path_buffer;
      path_buffer = nullptr;
    }
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
  }
};
