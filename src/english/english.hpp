#pragma once

#include "string.hpp"
#include "string_view.hpp"

namespace fast::english {

void trim_puntuation(fast::string &word);

void porter_stem(fast::string &word);

bool is_stop_word(fast::string_view word);

}
