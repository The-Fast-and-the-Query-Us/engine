#include <netdb.h>
#include <openssl/ssl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

static constexpr int MAX_PACKET_SIZE = 10240;
static constexpr uint16_t MAX_MESSAGE_SIZE = 8192;
static constexpr uint8_t REQUEST_TYPE_LEN = 16;
static constexpr uint8_t HOST_LEN = 6;
static constexpr uint8_t USER_AGENT_LEN = 38;
static constexpr uint8_t HTML_TAIL_LEN = 61;

class ParsedUrl {
 public:
  const char *CompleteUrl;
  char *Service, *Host, *Port, *Path;

  ParsedUrl(const char *url) {
    // Assumes url points to static text but
    // does not check.

    CompleteUrl = url;

    pathBuffer = new char[strlen(url) + 1];
    const char *f;
    char *t;
    for (t = pathBuffer, f = url; (*t++ = *f++););

    Service = pathBuffer;

    const char Colon = ':', Slash = '/';
    char *p;
    for (p = pathBuffer; *p && *p != Colon; p++);

    if (*p) {
      // Mark the end of the Service.
      *p++ = 0;

      if (*p == Slash) p++;
      if (*p == Slash) p++;

      Host = p;

      for (; *p && *p != Slash && *p != Colon; p++);

      if (*p == Colon) {
        // Port specified.  Skip over the colon and
        // the port number.
        *p++ = 0;
        Port = +p;
        for (; *p && *p != Slash; p++);
      } else
        Port = p;

      if (*p)
        // Mark the end of the Host and Port.
        *p++ = 0;

      // Whatever remains is the Path.
      Path = p;
    } else
      Host = Path = p;
  }

  ~ParsedUrl() { delete[] pathBuffer; }

 private:
  char *pathBuffer;
};

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " url" << '\n';
    // Safe return just for the sake of the html_parser test
    return 0;
  }

  // Parse the URL
  ParsedUrl url(argv[1]);

  // Get the host address.
  struct addrinfo *address{};
  struct addrinfo hints {};
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  if (getaddrinfo(url.Host, "443", &hints, &address) < 0) {
    std::cerr << "getaddrinfo failed. Address not found.\n";
    return 1;
  }

  // Create a TCP/IP socket.
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Connect the socket to the host address.
  if (connect(sockfd, address->ai_addr, address->ai_addrlen) < 0) {
    std::cerr << "Connection to host failed.\n";
    return 1;
  }

  // Build an SSL layer and set it to read/write
  // to the socket we've connected.
  if (SSL_library_init() < 0) {
    std::cerr << "Failed to initialize SSL library.\n";
    return 1;
  }

  SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
  if (!ctx) {
    std::cerr << "Failed to create SSL context.\n";
    close(sockfd);
    return 1;
  }

  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    std::cerr << "Failed to create SSL connection.\n";
    SSL_CTX_free(ctx);
    close(sockfd);
    return 1;
  }

  if (SSL_set_fd(ssl, sockfd) != 1) {
    std::cerr << "Failed to bind SSL to socket.\n";
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    return 1;
  }

  if (SSL_connect(ssl) != 1) {
    std::cerr << "SSL connection failed with error.\n";
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    return 1;
  }

  char get_request[MAX_MESSAGE_SIZE];
  char *p = get_request;
  char *limit = get_request + MAX_MESSAGE_SIZE;
  const char *agent_email = "shivgov@umich.edu";

  if (p + REQUEST_TYPE_LEN + strlen(url.Path) > limit) {
    std::cerr << "Get request buffer overflow.\n";
    return 1;
  }
  strcat(p, "GET /");
  if (strlen(url.Path)) {
    strcat(p, url.Path);
  }
  strcat(p, " HTTP/1.1\r\n");

  if (p + HOST_LEN + strlen(url.Host) + strlen(url.Port) > limit) {
    std::cerr << "Get request buffer overflow.\n";
    return 1;
  }
  strcat(p, "Host: ");
  strcat(p, url.Host);
  if (strlen(url.Port)) {
    strcat(p, url.Port);
  }
  strcat(p, "\r\n");

  if (p + USER_AGENT_LEN + strlen(agent_email) > limit) {
    std::cerr << "Get request buffer overflow.\n";
    return 1;
  }
  strcat(p, "User-Agent: LinuxGetUrl/2.0 ");
  strcat(p, agent_email);
  strcat(p, " (Linux)\r\n");

  if (p + HTML_TAIL_LEN > limit) {
    std::cerr << "Get request buffer overflow.\n";
    return 1;
  }
  strcat(
      p,
      "Accept: */*\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n");

  /*for (int i = 0; i < strlen(get_request); ++i) {*/
  /*  std::cout << get_request[i];*/
  /*}*/

  if (SSL_write(ssl, get_request, strlen(get_request)) <= 0) {
    std::cerr << "SSL write failed.\n";
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    return 1;
  }

  // Read from the socket until there's no more data, copying it to
  // stdout.
  char buffer[MAX_PACKET_SIZE];
  ssize_t bytes{};
  bool found_header_end = false;
  const char* header_end = "\r\n\r\n";
  int header_match_pos = 0;

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
    std::cerr << "SSL read failed with error.\n";
  }

  // Close the socket and free the address info structure.
  if (SSL_shutdown(ssl) < 0) {
    std::cerr << "SSL shutdown failed with error.\n";
  }
  SSL_free(ssl);
  close(sockfd);
  freeaddrinfo(address);
  SSL_CTX_free(ctx);
}
