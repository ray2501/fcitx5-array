/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
*/

#ifndef _FCITX5_ARRAY_HEADER_
#define _FCITX5_ARRAY_HEADER_

#include <string>
#include <vector>

extern "C" {
#include "sqlite3.h"
}

namespace Array {

class ArrayContext {
public:
    ArrayContext();
    ~ArrayContext();
    std::string get_preedit_string(std::string preedit);
    std::vector<std::string> get_candidates_from_main(std::string keys,
                                                      uint wildcard_char_count);
    std::vector<std::string> get_candidates_from_simple(std::string keys);
    std::vector<std::string> get_candidates_from_special(std::string keys);
    std::vector<std::string> get_candidates_from_phrase(std::string keys);
    std::vector<std::string>
    get_reverted_key_candidates_from_special(std::string ch);
    std::vector<std::string>
    get_reverted_char_candidates_from_special(std::string keys);
    bool input_key_is_not_special(std::string keys, std::string ch);

private:
    sqlite3 *conn;
};

} // namespace Array

#endif
