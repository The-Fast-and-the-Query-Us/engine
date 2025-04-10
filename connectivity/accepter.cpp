#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>
#include <iostream>

int main() {
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};

  addr.sin_port = htons(8080);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  int err = bind(sock_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr));
  if (err < 0) {
    std::cout << "error during bind\n";
    assert(false);
  }

  err = listen(sock_fd, SOMAXCONN);
  if (err < 0) {
    std::cout << "error during listen\n";
    assert(false);
  }

  std::cout << "Accepting...\n";
  int accept_fd = accept(sock_fd, nullptr, nullptr);
  std::cout << "Success\n";

  int res{};
  ssize_t bytes = recv(accept_fd, &res, sizeof(int), MSG_WAITALL);
  if (bytes < 0) {
    std::cout << "error during recv\n";
    assert(false);
  }
  std::cout << "recv'd " << res << '\n';
  close(accept_fd);
  close(sock_fd);
}
