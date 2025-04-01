#include <cassert>
#include <netinet/in.h>
#include <network.hpp>
#include <string.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

using namespace fast;

constexpr int PORT = 8080;
string words[] = {"abc", "smth else", "s", "long mf string adkjfbsdkfj", "", "medium"};

constexpr int NUM_WORDS = sizeof(words) / sizeof(string);

void *test(void*) {
  int sock = 0;
  struct sockaddr_in serv_addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  string buffer;
  for (int i = 0; i < NUM_WORDS; ++i) {
    recv_all(sock, buffer);
    assert(buffer == words[i]);
  }

  close(sock);
  return nullptr;
}

int main() {

  const auto server = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  bind(server, (sockaddr*) &addr, sizeof(addr));
  listen(server, 3);

  pthread_t client_thread;
  pthread_create(&client_thread, nullptr, test, nullptr);

  const auto client = accept(server, NULL, NULL);

  for (int i = 0; i < NUM_WORDS; ++i) {
    send_all(client, words[i]);
  }


  pthread_join(client_thread, NULL);
  close(client);
  close(server);
}
