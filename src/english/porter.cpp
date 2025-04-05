#include <string.hpp>
#include <english.hpp>

// return 1 shifted by how far this is from 'a'
// should only be called on lowercase letters
static constexpr int normalize(char c) {
  return 1 << (c - 'a');
}

/*

SOURCE = https://tartarus.org/martin/PorterStemmer/def.txt

A \consonant\ in a word is a letter other than A, E, I, O or U, and other
than Y preceded by a consonant. (The fact that the term `consonant' is
defined to some extent in terms of itself does not make it ambiguous.) So in
TOY the consonants are T and Y, and in SYZYGY they are S, Z and G. If a
letter is not a consonant it is a \vowel\.

*/
class char_classifier {
  bool last_was_cons = false;

public:

  bool is_vowel(char c) {
    switch (c) {
      case 'a':
      case 'e':
      case 'i':
      case 'o':
      case 'u':
        last_was_cons = false;
        return true;

      default:
        if (c == 'y' && last_was_cons) {
          last_was_cons = false;
          return true;
        } else {
          last_was_cons = true;
          return false;
        }
    }
  }
};

static int calc_m(const fast::string_view &word, size_t suffix_len) {
  if (suffix_len >= word.size()) return 0;

  char_classifier cc;
}

/*
* Rules:
*   SSES -> SS
*   IES  -> I
*   SS   -> SS
*   S    -> <NULL>
*/
static void porter_step_1a(fast::string &word) {
  if (word.ends_with("sses")) {
    word.pop_back(2);
  } else if (word.ends_with("ies")) {
    word.pop_back(2);
  } else if (word.ends_with("ss")) {
    // No-Op
  } else if (word.ends_with("s")) {
    word.pop_back();
  }
}

/*
* Rules:
*   (m > 0) EED -> EE
*   (*v*)   ED  -> <NULL>
*   (*v*)   ING -> <NULL>
*/
static void porter_step_1b(fast::string &word) {

}

void fast::english::porter_stem(fast::string &word) {
  porter_step_1a(word);
  porter_step_1b(word);
}
