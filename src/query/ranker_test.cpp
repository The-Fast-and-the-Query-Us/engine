#include "constants.hpp"
#include "vector.hpp"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <network.hpp>
#include <array.hpp>

using namespace fast;

constexpr unsigned QUERY_PORT = 8081;

int main(int argc, char **argv) {
  assert(argc == 2);

  const auto ip_file = fopen("ips.txt", "r");

  if (ip_file == NULL) {
    perror("Cannot find ip file");
  }

  fast::vector<int> servers;

  char buffer[40]{};
  while (fgets(buffer, sizeof(buffer), ip_file) != NULL) {
    buffer[strcspn(buffer, "\r\n")] = 0;

    if (buffer[0]) {
      const auto fd = socket(AF_INET, SOCK_STREAM, 0);

      if (fd < 0) {
        perror("Bad socket");
        exit(1);
      }

      struct sockaddr_in addr{};
      addr.sin_port = htons(QUERY_PORT);
      addr.sin_family = AF_INET;

      if (inet_pton(AF_INET, buffer, &addr.sin_addr) < 0) {
        perror("inet pton");
        close(fd);
        exit(1);
      }

      if (connect(fd, (sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("Server unreachable");
        close(fd);
        continue;
      }

      servers.push_back(fd);
    }
  }

  // send query
  const auto start = std::chrono::high_resolution_clock::now();

  for (const auto s : servers) {
    if (!fast::send_all(s, fast::string(argv[1]))) {
      std::cout << "Fail to send to query server" << std::endl;
    }
  }

  fast::array<fast::pair<fast::string, float>, 10> results;

  for (const auto s : servers) {
    uint32_t count;
    fast::recv_all(s, count);

    while (count--) {
      uint32_t rank;
      fast::string url;

      fast::recv_all(s, rank);
      fast::recv_all(s, url);



      float casted;
      memcpy(&casted, &rank, sizeof(casted));

      std::cout << "Recv: " << url.c_str() << " with rank: " << casted << std::endl;

      if (casted > results[9].second) {
        results[9] = {url, casted};

        for (size_t i = 9; i > 0; --i) {
          if (results[i].second > results[i - 1].second) {
            fast::swap(results[i], results[i - 1]);
          } else {
            break;
          }
        }
      }
    }
  }

  const auto end = std::chrono::high_resolution_clock::now();

  const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
    end - start
  );

  std::cout << "Time taken: " << time.count() << " ms" << std::endl;

  for (const auto &r : results) {
    std::cout << r.first.c_str() << " rank: " << r.second << std::endl;
  }
}
