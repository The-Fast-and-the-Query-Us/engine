#pragma once

#include <string.hpp>

namespace fast::english {

// not done yet
void trim_puntuation(fast::string &word);

// using 1980s original algorithm
// should probably add modern optimizations before using
void porter_stem(fast::string &word);

void normalize_text(fast::string &word);

// not done yet
bool is_word(const fast::string &word);

// not done yet
bool is_stop_word(const fast::string &word);

}
