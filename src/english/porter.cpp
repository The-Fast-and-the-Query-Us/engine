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

  bool last_was_vowel() const { return !last_was_cons; }

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

/*
A consonant will be denoted by c, a vowel by v. A list ccc... of length
greater than 0 will be denoted by C, and a list vvv... of length greater
than 0 will be denoted by V. Any word, or part of a word, therefore has one
of the four forms:

    CVCV ... C
    CVCV ... V
    VCVC ... C
    VCVC ... V

These may all be represented by the single form

    [C]VCVC ... [V]

where the square brackets denote arbitrary presence of their contents.
Using (VC){m} to denote VC repeated m times, this may again be written as

    [C](VC){m}[V].

m will be called the \measure\ of any word or word part when represented in
this form. The case m = 0 covers the null word. Here are some examples:

    m=0    TR,  EE,  TREE,  Y,  BY.
    m=1    TROUBLE,  OATS,  TREES,  IVY.
    m=2    TROUBLES,  PRIVATE,  OATEN,  ORRERY.
*/
static int calc_m(const fast::string_view &word) {
  char_classifier cc;
  int m = 0;

  for (const auto c : word) {
    if (cc.last_was_vowel() && !cc.is_vowel(c)) ++m;
  }

  return m;
}

// the stem ends with S (and similarly for the other letters)
static bool s_star(const fast::string_view &word, char s) {
  return word.size() > 0 && word[word.size() - 1] == s;
}

// the stem contains a vowel
static bool v_star(const fast::string_view &word) {
  char_classifier cc;
  for (const auto c : word) {
    if (cc.is_vowel(c)) return true;
  }
  return false;
}

// the stem ends with a double consonant (e.g. -TT, -SS)
static bool d_star(const fast::string_view &word) {
  if (word.size() < 2) return false;

  char_classifier cc;
  if (word.size() > 2) cc.is_vowel(word[word.size() - 3]);

  return (
    !cc.is_vowel(word[word.size() - 2]) &&
    !cc.is_vowel(word[word.size() - 1])
  );
}

// the stem ends cvc, where the second c is not W, X or Y (e.g. -WIL, -HOP)
static bool o_star(const fast::string_view &word) {
  if (word.size() < 3) return false;

  switch (word[word.size() - 1]) {
    case 'w':
    case 'x':
    case 'y':
      return false;
  }

  char_classifier cc;
  // init for special case of y
  if (word.size() > 3) cc.is_vowel(word[word.size() - 4]);

  return (
    !cc.is_vowel(word[word.size() - 3]) &&
    cc.is_vowel(word[word.size() - 2]) &&
    !cc.is_vowel(word[word.size() - 1])
  );
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
Rules:
   (m > 0) EED -> EE
   (*v*)   ED  -> <NULL>
   (*v*)   ING -> <NULL>
   
   If the second or third of the rules in Step 1b is successful, the following is performed.

      AT -> ATE
      BL -> BLE
      IZ ->IZE
      (*d and not (*L or *S or *Z)) -> single letter
      (m=1 and *o) -> E
*/
static void porter_step_1b(fast::string &word) {
  if (word.ends_with("eed") && calc_m(fast::string_view(word).trim_suffix(3)) > 0) {
    word.pop_back();
    return;
  } else if (word.ends_with("ed") && v_star(fast::string_view(word).trim_suffix(2))) {
    word.pop_back(2);
  } else if (word.ends_with("ing") && v_star(fast::string_view(word).trim_suffix(3))) {
    word.pop_back(3);
  } else {
    return;
  }

  if (word.ends_with("at")) {
    word += 'e';
  } else if (word.ends_with("bl")) {
    word += 'e';
  } else if (d_star(word) && !(s_star(word, 'l') || s_star(word, 's') || s_star(word, 'z'))) {
    word.pop_back();
  } else if (calc_m(word) == 1 && o_star(word)) {
    word += 'e';
  }
}

void fast::english::porter_stem(fast::string &word) {
  porter_step_1a(word);
  porter_step_1b(word);
}
