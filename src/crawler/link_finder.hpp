#pragma once

#include <stdexcept>
#include "../lib/string.hpp"
#include "../lib/vector.hpp"
#include "url_parser.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <netdb.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>

static constexpr int MAX_PACKET_SIZE = 10240;
static constexpr uint16_t MAX_MESSAGE_SIZE = 8192;
static constexpr uint8_t REQUEST_TYPE_LEN = 16;
static constexpr uint8_t HOST_LEN = 6;
static constexpr uint8_t USER_AGENT_LEN = 38;
static constexpr uint8_t HTML_TAIL_LEN = 61;

class link_finder {
  public:
    link_finder(const char *file) : url_parts(), sock_fd(), url(file), ctx(), ssl()  {
      file_fd = open(url, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (file_fd == -1) {
        throw std::runtime_error("Could not open file in link_finder\n");
      }
    } 

    ~link_finder() {
      SSL_free(ssl);
      close(sock_fd);
      freeaddrinfo(address);
      SSL_CTX_free(ctx);
      close(file_fd);
    }

    fast::vector<fast::string> extract_links() {
      setup_connection();
      send_get_request();

      fast::vector<fast::string> links;

      char buffer[MAX_PACKET_SIZE];
      ssize_t bytes{};
      bool found_header_end = false;
      const char* header_end = "\r\n\r\n";
      int header_match_pos = 0;

      // TODO: Need to write to a file.
      while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
        if (!found_header_end) {
          for (int i = 0; i < bytes; i++) {
            if (buffer[i] == header_end[header_match_pos]) {
              header_match_pos++;
              if (header_match_pos == 4) {  // Found \r\n\r\n
                found_header_end = true;
                write(1, buffer + i + 1, bytes - (i + 1));
                break;
              }
            } else {
              header_match_pos = (buffer[i] == '\r') ? 1 : 0;
            }
          }
        } else {
          write(1, buffer, bytes);
        }
      }
      if (bytes < 0) {
        throw std::runtime_error("SSL read failed with error.\n");
      }

      if (SSL_shutdown(ssl) < 0) {
        throw std::runtime_error("SSL shutdown failed with error.\n");
      }
      
      // TODO: Currently this function just dumps the html to std::cout.
      // We need to parse packet by packet and extract the links if there is no robots.txt
      // If there is, then use that (and cache that info) to crawl those links

      return links;
    }
    // We want to name the html file the URL
    // Find all the links in each packet we receive. If we are still in a link across a packet we can continue to the next packet
    // write each packet to the file correctly

  private:
    struct addrinfo *address{};
    struct addrinfo hints {};
    url_parser url_parts;
    int file_fd, sock_fd;
    const char *url;
    SSL_CTX *ctx;
    SSL *ssl;

    void setup_connection() {
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      if (getaddrinfo(url_parts.host, "443", &hints, &address) < 0) {
        throw std::runtime_error("getaddrinfo failed. Address not found.\n");
      }

      sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

      if (connect(sock_fd, address->ai_addr, address->ai_addrlen) < 0) {
        throw std::runtime_error("Connection to host failed.\n");
      }

      if (SSL_library_init() < 0) {
        throw std::runtime_error("Failed to initialize SSL library.\n");
      }

      ctx = SSL_CTX_new(SSLv23_client_method());
      if (!ctx) {
        close(sock_fd);
        throw std::runtime_error("Failed to create SSL context.\n");
      }

      ssl = SSL_new(ctx);
      if (!ssl) {
        SSL_CTX_free(ctx);
        close(sock_fd);
        throw std::runtime_error("Failed to create SSL connection.\n");
      }

      if (SSL_set_fd(ssl, sock_fd) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock_fd);
        throw std::runtime_error("Failed to bind SSL to socket.\n");
      }

      if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock_fd);
        throw std::runtime_error("SSL connection failed with error.\n");
      }
    }

    void send_get_request() {
      char get_request[MAX_MESSAGE_SIZE];
      char *p = get_request;
      char *limit = get_request + MAX_MESSAGE_SIZE;
      const char *agent_email = "shivgov@umich.edu";

      if (p + REQUEST_TYPE_LEN + strlen(url_parts.path) > limit) {
        throw std::runtime_error("Get request buffer overflow.\n");
      }
      strcat(p, "GET /");
      if (strlen(url_parts.path)) {
        strcat(p,url_parts.path);
      }
      strcat(p, " HTTP/1.1\r\n");

      if (p + HOST_LEN + strlen(url_parts.host) + strlen(url_parts.port) > limit) {
        throw std::runtime_error("Get request buffer overflow.\n");
      }
      strcat(p, "host: ");
      strcat(p,url_parts.host);
      if (strlen(url_parts.port)) {
        strcat(p,url_parts.port);
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
      strcat(p, "Accept: */*\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n");

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
};
