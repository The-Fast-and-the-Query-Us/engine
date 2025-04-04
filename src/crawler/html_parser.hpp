// HtmlParser.h
// Nicole Hamilton, nham@umich.edu

#pragma once

#include "html_tags.hpp"
#include "string.hpp"
#include "vector.hpp"

// This is a simple HTML parser class.  Given a text buffer containing
// a presumed HTML page, the constructor will parse the text to create
// lists of words, title words and outgoing links found on the page.  It
// does not attempt to parse the entire the document structure.
//
// The strategy is to word-break at whitespace and HTML tags and discard
// most HTML tags.  Three tags require discarding everything between
// the opening and closing tag. Five tags require special processing.
//
// We will use the list of possible HTML element names found at
// https://developer.mozilla.org/en-US/docs/Web/HTML/Element +
// !-- (comment), !DOCTYPE and svg, stored as a table in HtmlTags.h.

// Here are the rules for recognizing HTML tags.
//
// 1. An HTML tag starts with either < if it's an opening tag or </ if
//    it's closing token.  If it starts with < and ends with /> it is both.
//
// 2. The name of the tag must follow the < or </ immediately.  There can't
//    be any whitespace.
//
// 3. The name is terminated by whitespace, > or / and is case-insensitive.
//
// 4. If it is terminated by whitepace, arbitrary text representing various
//    arguments may follow, terminated by a > or />.
//
// 5. If the name isn't on the list we recognize, we assume it's the whole
//    is just ordinary text.
//
// 6. Every token is taken as a word-break.
//
// 7. Most opening or closing tokens can simply be discarded.
//
// 8. <script>, <style>, and <svg> require discarding everything between the
//    opening and closing tag.  Unmatched closing tags are discarded.
//
// 9. <!--, <title>, <a>, <base> and <embed> require special processing.
//
//      <-- is the beginng of a comment.  Everything up to the ending -->
//          is discarded.
//
//      <title> should cause all the words between the opening and closing
//          tags to be added to the titleWords vector rather than the default
//          words vector.  A closing </title> without an opening <title> is
//          discarded.
//
//      <a> is expected to contain an href="...url..."> argument with the
//          URL inside the double quotes that should be added to the list
//          of links.  All the words in between the opening and closing tags
//          should be collected as anchor text associated with the link
//          in addition to being added to the words or titleWords vector,
//          as appropriate.  A closing </a> without an opening <a> is
//          discarded.
//
//     <base> may contain an href="...url..." parameter.  If present, it should
//          be captured as the base variable.  Only the first is recognized; any
//          others are discarded.
//
//     <embed> may contain a src="...url..." parameter.  If present, it should
//     be
//          added to the links with no anchor text.

class Link {
public:
  fast::string URL;
  fast::vector<fast::string> anchorText;

  Link(fast::string &URL) : URL(URL) {}
};

class HtmlParser {
private:
  static bool IsWhitespace(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f');
  }

  static bool StrEqual(const char *ls, const char *le, const char *rs,
                       const char *re) {
    if (le - ls != re - rs)
      return false;
    while (ls != le) {
      if (*(ls++) != *(rs++))
        return false;
    }
    return true;
  }

  static const char *NameEnd(const char *buff, const char *end) {
    while (buff != end && !IsWhitespace(*buff) && *buff != '/' && *buff != '>')
      ++buff;
    return buff;
  }

  // return one past the end of the tag @ buf
  static const char *TagEnd(const char *buff, const char *end) {
    while (buff != end && *(buff++) != '>')
      ;
    return buff;
  }

  // return pointer to '<' in the tags pair
  static const char *TagPair(const char *name, const char *name_end,
                             const char *buff, const char *end) {
    while (buff != end) {
      if (buff[0] != '<')
        ++buff;
      else if (buff[1] != '/')
        ++buff; // Check bounds?
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
  fast::vector<fast::string> words, titleWords;
  fast::vector<Link> links;
  fast::string base;

  HtmlParser(const char *buffer, size_t length) {
    const char *start = buffer;
    const char *const end = buffer + length;

    bool inAnchor = false;
    bool inTitle = false;

    bool base_set = false;

    while (buffer != end) {
      if (IsWhitespace(buffer[0])) {
        if (start != buffer) {
          if (inTitle)
            titleWords.emplace_back(start, buffer);
          else
            words.emplace_back(start, buffer);
          if (inAnchor)
            links.back().anchorText.emplace_back(start, buffer);
        };

        ++buffer;
        start = buffer;
      } else if (buffer[0] == '<') {
        if (buffer[1] == '/') {
          const auto name_end = NameEnd(buffer + 2, end);
          const auto action = LookupPossibleTag(buffer + 2, name_end);

          if (action != DesiredAction::OrdinaryText) {
            if (start != buffer) {
              if (inTitle)
                titleWords.emplace_back(start, buffer);
              else
                words.emplace_back(start, buffer);
              if (inAnchor)
                links.back().anchorText.emplace_back(start, buffer);
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
            ++buffer; // buffer = name_end
            continue;
          } else if (start != buffer) {
            if (inTitle)
              titleWords.emplace_back(start, buffer);
            else
              words.emplace_back(start, buffer);

            if (inAnchor)
              links.back().anchorText.emplace_back(start, buffer);
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
                int i; // href="
                for (i = 0; i < 6; ++i) {
                  if (buffer[i] != "href=\""[i])
                    break;
                }

                if (i == 6) {
                  auto ep = buffer + 6;
                  while (*ep != '\"')
                    ++ep;
                  base = fast::string(buffer + 6, ep);
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
              int i; // href="
              for (i = 0; i < 5; ++i) {
                if (buffer[i] != "src=\""[i])
                  break;
              }

              if (i == 5) {
                auto ep = buffer + 5;
                while (*ep != '\"')
                  ++ep;
                links.emplace_back(fast::string(buffer + 5, ep));
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
              int i; // href="
              for (i = 0; i < 6; ++i) {
                if (buffer[i] != "href=\""[i])
                  break;
              }

              if (i == 6) {
                auto ep = buffer + 6;
                while (*ep != '\"')
                  ++ep;
                links.emplace_back(fast::string(buffer + 6, ep));
                buffer = ep;
                break;
              }
              ++buffer;
            }
            if (*buffer != '>')
              inAnchor = true;
            buffer = TagEnd(buffer, end);
            start = buffer;
            break;
          case DesiredAction::Comment:
            while (true) {
              int i;
              for (i = 0; i < 3; ++i) {
                if (buffer[i] != "-->"[i])
                  break;
              }

              if (i == 3)
                break;
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
      words.emplace_back(start, buffer);
    }
  }
};
