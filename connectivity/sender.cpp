#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
  const int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr{};

  addr.sin_port = htons(8080);
  addr.sin_family = AF_INET;

  inet_pton(AF_INET, argv[1], &addr.sin_addr);

  std::cout << "Here" << std::endl;

  connect(fd, (sockaddr *) &addr, sizeof(addr));

  int yes = 1;
  send(fd, (void*) &yes, sizeof(int), 0);

  std::cout << "Good" << std::endl;
  close(fd);
}

