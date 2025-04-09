#include <iostream>
#include <cstdlib>
#include <html_file.hpp>
#include <html_parser.hpp>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  int result = system("./test_ssl_getter https://www.nytimes.com > nyt.html");
  if (result == -1) return 1;
  int fd = open("nyt.html", O_RDONLY);
  if (fd == -1) return 1;

  auto *buffer = new char[1'000'000]; // NOLINT
  ssize_t bytes{};
  struct stat st{};
  if (fstat(fd, &st) < 0) {
    perror("fstat");
    close(fd);
    delete[] buffer;
    return 1;
  }
  off_t file_size = st.st_size;

  bytes = read(fd, buffer, file_size);
  if (bytes < 0) {
    close(fd);
    delete[] buffer;
    return 1;
  }

  html_parser parser(buffer, bytes);
  std::cout << "Titles: " << '\n';
  for (const auto &title : parser.titleWords) {
    std::cout << title.begin() << '\n';
  }

  std::cout << "Words:" << '\n';
  for (const auto &word : parser.words) {
    std::cout << word.begin() << '\n';
  }

  std::cout << "Link:" << std::endl;
  for (auto &link : parser.links) {
    std::cout << link.URL.begin() << '\n';
  }

  close(fd);
  delete[] buffer;
  return 0;

}
