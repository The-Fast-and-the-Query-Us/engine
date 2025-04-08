#include <cstddef>
#include <iostream>
#include <cassert>
#include "string.hpp"

fast::string blacklist[150] = {
  "login", "signin", "signup", "account", "password", "admin", "profile",
  "cart", "checkout", "buy", "purchase", "register", "payment", "shop", "order",
  "cdn", "static", "assets", "media", "content", "cache", 

  "fr", "es", "de", "it", "ru", "ja", "zh", "ko", "ar", "pt", "nl", "pl", "tr",
  "bg", "cs", "da", "el", "fi", "he", "hi", "hu", "id", "no", "ro", "sk", "sl",
  "sv", "th", "vi", "hr", "ca", "et", "fa", "lt", "lv", "ms", "sr", "sw",
  "tl", "ur", "bn", "bs", "cy", "eo", "eu", "gl", "hy", "is", "ka", "kk", "km",
  "kn", "ky", "lo", "mk", "ml", "mn", "mr", "mt", "my", "ne", "si", "sq", "ta",
  "te", "uz", "zu", "cn", "jp", "kr", "ru", "tw", "hk", "br", "mx", "ar", "cl", "pe", "ve", "za",
  "ae", "sa", "sg", "in", "pk", "ph", "vn", "th", "my", "id", "il", "tr", "ir",

  "baidu", "weibo", "qq", "taobao", "yandex", "vk", "mail.ru", "rambler", 
  "naver", "daum", "yahoo.co.jp", "goo.ne.jp", "mercadolibre",
  "allegro", "coccoc", "onet", "interia", "wp", "sohu", "sina", 
  "alipay", "tencent", "bilibili", "youku", "tudou", "dmm", "kakao", "line",
  "mixi", "nicovideo", "pixiv", "qzone", "renren", "wechat", "weixin"
};

bool is_blacklisted(const fast::string &url) {
  const char* word_start = nullptr;
  const char* url_end = url.begin() + url.size();

  for (const char* p = url.begin(); p <= url_end; ++p) {
    bool is_delimiter = (p == url_end) || 
      (*p == '/') || (*p == '.') || (*p == '#') || 
      (*p == '?') || (*p == '&') || (*p == '=') || 
      (*p == '-') || (*p == '_') || (*p == ':');

    if (is_delimiter) {
      if (word_start && p > word_start) {
        size_t word_len = p - word_start;

        if (word_len > 1) { // skip single letters
          for (const auto& banned : blacklist) {
            if (banned.size() == word_len && 
              strncmp(word_start, banned.begin(), word_len) == 0) {
              for (size_t i = 0; i < word_len; ++i) {
                std::cout << word_start[i];
              }
              return true;
            }
          }
        }
      }
      word_start = nullptr;
    } else if (!word_start) {
      word_start = p;
    }
  }

  return false;
}

int main() {
  fast::pair<fast::string, bool> test_urls[] = {
    {"lskhflksdflsdk.#.slkfdjloginskldjf", false},
    {"www.hello.html?query=login", true},
    {"https://www.nytimes.com", false},
    {"cn.nyt.com", true},
    {"data.cdn#title", true},
    {"wtfyoushearchingthissite.com/?id=79/?hello", false},
    {"es.wikipedia.com", true}
  };

  for (const auto &pair : test_urls) {
    std::cout << pair.first.begin() << '\n';
    assert(is_blacklisted(pair.first) == pair.second);
  }

  return 0;
}
