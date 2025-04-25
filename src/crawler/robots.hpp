#pragma once
#include "string.hpp"
#include "vector.hpp"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdexcept>
#include <utility>

namespace fast {

#define ROBOTS_MAX_SIZE 1024 * 16

class robots_filter {

  class rule {
    friend class robots_filter;
    enum mode {
      ALLOW,
      DISALLOW,
    };

    string pattern;

  public:
    mode allow;

    rule(bool _allow, const string &_pat)
        : pattern{_pat}, allow{_allow ? ALLOW : DISALLOW} {}
    rule() : pattern{""}, allow{ALLOW} {}

    bool match(string url) const {
      // find the start of the route in the url
      size_t start = url.find('/');

      // something wrong with url
      if (start == url.size() - 1)
        return false;

      // that / was part of the http://
      if (url[start] + 1 == '/')
        start = url.find('/', start + 2);

      string route = url.substr(start);

      if (route == pattern || (route.size() > pattern.size() &&
                               route.substr(0, pattern.size()) == pattern &&
                               route[pattern.size()] == '/'))
        return true;

      return false;
    }
  };
  // class rule

  string domain;
  vector<rule> rules;

  static void init_openssl() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
  }

  static void cleanup_openssl() {
    ERR_free_strings();
    EVP_cleanup();
  }

  string read_from_bio(BIO *bio, size_t max_size) {
    string res;

    char buf[1024];
    while (res.size() < max_size) {
      // only ask for as much as will fit
      int to_read = static_cast<int>(
          std::min(static_cast<size_t>(sizeof(buf)), max_size - res.size()));
      int n = BIO_read(bio, buf, to_read);
      if (n > 0) {
        res.append(buf, n);
        continue;
      }
      // n == 0: clean EOF
      if (n == 0) {
        break;
      }

      if (BIO_should_retry(bio)) {
        // no data at this second just wait
        continue;
      }
      // bad error
      break;
    }

    return res;
  }

  string fetch_robots(const string &domain) {
    init_openssl();

    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
      throw std::runtime_error("SSL_CTX_new() failed\n");
    }

    // Create a BIO that does both connect + SSL
    BIO *bio = BIO_new_ssl_connect(ctx);
    if (!bio) {
      SSL_CTX_free(ctx);
      throw std::runtime_error("BIO_new_ssl_connect() failed\n");
    }

    // Set host:port
    string target = domain + ":443";
    BIO_set_conn_hostname(bio, target.c_str());

    // Enable SNI
    SSL *ssl = nullptr;
    BIO_get_ssl(bio, &ssl);
    if (!ssl) {
      BIO_free_all(bio);
      SSL_CTX_free(ctx);
      throw std::runtime_error("BIO_get_ssl() failed\n");
    }
    SSL_set_tlsext_host_name(ssl, domain.c_str());

    // Connect TCP + handshake
    if (BIO_do_connect(bio) <= 0) {
      BIO_free_all(bio);
      SSL_CTX_free(ctx);
      string err = "Connection to " + target + " failed\n";
      throw std::runtime_error(err.c_str());
    }

    // verify cert
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
      std::cerr << "Certificate verification error: "
                << SSL_get_verify_result(ssl) << "\n";
    }

    // Build HTTP GET request
    string req = "GET /robots.txt HTTP/1.1\r\n"
                 "Host: " +
                 domain +
                 "\r\n"
                 "User-Agent: LinuxGetparser.2.0\r\n"
                 "Connection: close\r\n"
                 "\r\n";

    if (BIO_write(bio, req.c_str(), (int)req.size()) <= 0) {
      BIO_free_all(bio);
      SSL_CTX_free(ctx);
      throw std::runtime_error("BIO_write() failed\n");
    }

    string res = read_from_bio(bio, ROBOTS_MAX_SIZE);

    // Cleanup
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    cleanup_openssl();

    return res;
  }

  string get_line(const string &buf, size_t offset) {
    size_t N = buf.size();
    // go to beginning of line
    while (offset < N && (buf[offset] == '\r' || buf[offset] == '\n'))
      ++offset;

    string res;
    while (offset < N && buf[offset] != '\r' && buf[offset] != '\n') {
      res += buf[offset++];
    }

    return res;
  }

  enum parse_action {
    WAIT_UNTIL_USER,
    USE_RULE,
  };

  vector<rule> parse_robots(const string &robots_text) {
    string line;
    size_t offset = 0;
    parse_action action = WAIT_UNTIL_USER;
    vector<rule> res;
    while (true) {
      offset += line.size();
      line = get_line(robots_text, offset);

      switch (action) {
      case WAIT_UNTIL_USER:
        if (line == "User-agent: *") {
          action = USE_RULE;
          continue;
        }
        break;
      case USE_RULE:
        size_t col_idx = line.find(':');
        if (col_idx < 0)
          continue;
        string prefix = line.substr(0, col_idx);
        if (prefix != "Allow" && prefix != "Disallow") {
          if (prefix.substr(0, 10) == "User-agent")
            action = WAIT_UNTIL_USER;
        }
        string pattern = line.substr(col_idx + 2, line.size() - col_idx - 2);
        if (pattern.size() == 0)
          continue;
        rules.push_back(rule(prefix == "Allow" ? true : false, pattern));
        break;
      }
    }
  }

public:
  robots_filter(const string &domain) {
    rules = parse_robots(fetch_robots(domain));
  }

  robots_filter(const robots_filter &other) { rules = other.rules; }

  robots_filter(robots_filter &&other) { rules = std::move(other.rules); }

  bool allow(const string &url) {

    bool allow_url = true;

    for (const auto &r : rules) {
      if (r.match(url)) {
        allow_url = r.allow == rule::ALLOW;
      }
    }

    return allow_url;
  }
};
} // namespace fast
