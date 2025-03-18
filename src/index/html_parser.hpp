#pragma once

#include <hashtable.hpp>
#include <string.hpp>
#include <vector.hpp>

#include "html_tags.hpp"

namespace fast {

class Link {
 public:
  string URL;
  vector<string> anchorText;

  Link(string URL) : URL(URL) {}
};  // namespace fastclass Link

class HtmlParser {
 public:
  // vector<string> words, titleWords;
  // vector<Link> links;
  // string base;

 private:
  static bool IsWhitespace(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f');
  }

  static bool StrEqual(const char* ls, const char* le, const char* rs,
                       const char* re) {
    if (le - ls != re - rs) return false;
    while (ls != le) {
      if (*(ls++) != *(rs++)) return false;
    }
    return true;
  }

  static const char* NameEnd(const char* buff, const char* end) {
    while (buff != end && !IsWhitespace(*buff) && *buff != '/' && *buff != '>')
      ++buff;
    return buff;
  }

  // return one past the end of the tag @ buf
  static const char* TagEnd(const char* buff, const char* end) {
    while (buff != end && *(buff++) != '>');
    return buff;
  }

  // return pointer to '<' in the tags pair
  static const char* TagPair(const char* name, const char* name_end,
                             const char* buff, const char* end) {
    while (buff != end) {
      if (buff[0] != '<')
        ++buff;
      else if (buff[1] != '/')
        ++buff;  // Check bounds?
      else {
        const auto this_end = NameEnd(buff + 2, end);

        if (StrEqual(name, name_end, buff + 2, this_end)) {
          return buff;
        } else {
          ++buff;
        }
      }
    }
    return end;
  }

 public:
  // The constructor is given a buffer and length containing
  // presumed HTML.  It will parse the buffer, stripping out
  // all the HTML tags and producing the list of words in body,
  // words in title, and links found on the page.

  HtmlParser(const char* buffer, size_t length, hashtable& index) {
    const char* start = buffer;
    const char* const end = buffer + length;

    bool inAnchor = false;
    bool inTitle = false;

    bool base_set = false;

    while (buffer != end) {
      if (IsWhitespace(buffer[0])) {
        if (start != buffer) {
          if (inTitle) {
            string word(start, buffer);
            index.add(word);
            // titleWords.emplace_back(start, buffer);
          } else {
            string word(start, buffer);
            index.add(word);
            // words.emplace_back(start, buffer);
          }
          // if (inAnchor) links.back().anchorText.emplace_back(start, buffer);
        };

        ++buffer;
        start = buffer;
      } else if (buffer[0] == '<') {
        if (buffer[1] == '/') {
          const auto name_end = NameEnd(buffer + 2, end);
          const auto action = LookupPossibleTag(buffer + 2, name_end);

          if (action != DesiredAction::OrdinaryText) {
            if (start != buffer) {
              if (inTitle) {
                string word(start, buffer);
                index.add(word);
                // titleWords.emplace_back(start, buffer);
              } else {
                string word(start, buffer);
                index.add(word);
                // words.emplace_back(start, buffer);
              }
              // if (inAnchor) links.back().anchorText.emplace_back(start,
              // buffer);
            }

            buffer = TagEnd(name_end, end);
            start = buffer;
          } else {
            ++buffer;
          }

          if (action == DesiredAction::Anchor)
            inAnchor = false;
          else if (action == DesiredAction::Title)
            inTitle = false;
        } else {
          const auto name_end = NameEnd(buffer + 1, end);
          const auto action = LookupPossibleTag(buffer + 1, name_end);

          if (action == DesiredAction::OrdinaryText) {
            ++buffer;  // buffer = name_end
            continue;
          } else if (start != buffer) {
            if (inTitle) {
              string word(start, buffer);
              index.add(word);
              // titleWords.emplace_back(start, buffer);
            } else {
              string word(start, buffer);
              index.add(word);
              // words.emplace_back(start, buffer);
            }

            // if (inAnchor) links.back().anchorText.emplace_back(start,
            // buffer);
          }

          switch (action) {
            case DesiredAction::Discard:
              buffer = TagEnd(name_end, end);
              start = buffer;
              break;
            case DesiredAction::DiscardSection:
              buffer = TagPair(buffer + 1, name_end, name_end, end);
              buffer = TagEnd(buffer, end);
              start = buffer;
              break;
            case DesiredAction::Title: {
              buffer = TagEnd(name_end, end);
              start = buffer;
              inTitle = true;
            } break;
            case DesiredAction::Base:
              if (!base_set) {
                buffer = name_end;
                while (*buffer != '>') {
                  int i;  // href="
                  for (i = 0; i < 6; ++i) {
                    if (buffer[i] != "href=\""[i]) break;
                  }

                  if (i == 6) {
                    auto ep = buffer + 6;
                    while (*ep != '\"') ++ep;
                    // base = string(buffer + 6, ep);
                    buffer = ep;
                    base_set = true;
                    break;
                  }
                  ++buffer;
                }
              }
              buffer = TagEnd(buffer, end);
              start = buffer;
              break;
            case DesiredAction::Embed: {
              buffer = name_end;
              while (*buffer != '>') {
                int i;  // href="
                for (i = 0; i < 5; ++i) {
                  if (buffer[i] != "src=\""[i]) break;
                }

                if (i == 5) {
                  auto ep = buffer + 5;
                  while (*ep != '\"') ++ep;
                  // links.emplace_back(string(buffer + 5, ep));
                  buffer = ep;
                  break;
                }
                ++buffer;
              }
              buffer = TagEnd(buffer, end);
              start = buffer;
            } break;
            case DesiredAction::Anchor:
              buffer = name_end;
              while (*buffer != '>') {
                int i;  // href="
                for (i = 0; i < 6; ++i) {
                  if (buffer[i] != "href=\""[i]) break;
                }

                if (i == 6) {
                  auto ep = buffer + 6;
                  while (*ep != '\"') ++ep;
                  // links.emplace_back(string(buffer + 6, ep));
                  buffer = ep;
                  break;
                }
                ++buffer;
              }
              if (*buffer != '>') inAnchor = true;
              buffer = TagEnd(buffer, end);
              start = buffer;
              break;
            case DesiredAction::Comment:
              while (true) {
                int i;
                for (i = 0; i < 3; ++i) {
                  if (buffer[i] != "-->"[i]) break;
                }

                if (i == 3) break;
                ++buffer;
              }
              buffer = TagEnd(buffer, end);
              start = buffer;
              break;
            default:
              buffer = TagEnd(name_end, end);
              start = buffer;
              break;
          }
        }
      } else {
        ++buffer;
      }
    }

    if (buffer != start) {
      string word(start, buffer);
      index.add(word);
      // words.emplace_back(start, buffer);
    }
  }
};

}  // namespace fast
