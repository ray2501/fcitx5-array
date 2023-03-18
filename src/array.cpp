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

#include <fcitx-utils/standardpath.h>
#include <string>
#include <vector>

#include "array.h"

namespace Array {

const char *valid_key_map[] = {
    "1-", // a
    "5v", // b
    "3v", // c
    "3-", // d
    "3^", // e
    "4-", // f
    "5-", // g
    "6-", // h
    "8^", // i
    "7-", // j
    "8-", // k
    "9-", // l
    "7v", // m
    "6v", // n
    "9^", // o
    "0^", // p
    "1^", // q
    "4^", // r
    "2-", // s
    "5^", // t
    "7^", // u
    "4v", // v
    "2^", // w
    "2v", // x
    "6^", // y
    "1v", // z
    "8v", //,
    "9v", //.
    "0v", ///
    "0-", //;
    " ?", //?
    " *", //*
};

ArrayContext::ArrayContext() {
    std::string FILE_PATH = fcitx::StandardPath::global().locate(
        fcitx::StandardPath::Type::PkgData, "array/array.db");

    if (FILE_PATH.empty()) {
        FCITX_INFO() << "Error: not found array.db!!!";
    } else {
        FCITX_INFO() << "OK: found array.db!!!";
    }

    if (sqlite3_open(FILE_PATH.c_str(), &(this->conn)) != SQLITE_OK) {
        this->conn = NULL;
    }
}

ArrayContext::~ArrayContext() {
    if (this->conn != NULL) {
        sqlite3_close(this->conn);
    }
}

std::string ArrayContext::get_preedit_string(std::string preedit) {
    std::string result;

    unsigned int i;
    for (i = 0; i < preedit.length(); i++) {
        char c = preedit[i];
        int index = -1;

        if (c >= 'a' && c <= 'z') {
            index = c - 'a';
        } else if (c == ',') {
            index = 26;
        } else if (c == '.') {
            index = 27;
        } else if (c == '/') {
            index = 28;
        } else if (c == ';') {
            index = 29;
        } else if (c == '?') {
            index = 30;
        } else if (c == '*') {
            index = 31;
        }

        if (index >= 0) {
            result.append(valid_key_map[index]);
        }
    }

    return result;
}

std::vector<std::string>
ArrayContext::get_candidates_from_main(std::string keys,
                                       uint wildcard_char_count) {
    std::vector<std::string> result;
    sqlite3_stmt *stmt;

    int retcode;
    if (!wildcard_char_count)
        retcode = sqlite3_prepare_v2(
            this->conn, "SELECT ch FROM main WHERE keys=?", -1, &stmt, NULL);
    else
        retcode = sqlite3_prepare_v2(this->conn,
                                     "SELECT ch FROM main WHERE keys GLOB ?",
                                     -1, &stmt, NULL);

    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *ch = (const char *)sqlite3_column_text(stmt, 0);
            result.push_back(std::string(ch));
        }
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

std::vector<std::string>
ArrayContext::get_candidates_from_simple(std::string keys) {
    std::vector<std::string> result;
    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(
        this->conn, "SELECT ch FROM simple WHERE keys=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *ch = (const char *)sqlite3_column_text(stmt, 0);
            result.push_back(std::string(ch));
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

std::vector<std::string>
ArrayContext::get_candidates_from_special(std::string keys) {
    std::vector<std::string> result;
    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(this->conn,
                                 "SELECT ch FROM main WHERE cat='2' AND keys=?",
                                 -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *ch = (const char *)sqlite3_column_text(stmt, 0);
            result.push_back(std::string(ch));
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

std::vector<std::string>
ArrayContext::get_candidates_from_phrase(std::string keys) {
    std::vector<std::string> result;
    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(
        this->conn, "SELECT ph FROM phrase WHERE keys=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *ph = (const char *)sqlite3_column_text(stmt, 0);
            result.push_back(std::string(ph));
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

std::vector<std::string>
ArrayContext::get_reverted_key_candidates_from_special(std::string ch) {
    std::vector<std::string> result;
    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(this->conn,
                                 "SELECT keys FROM main WHERE cat='2' AND ch=?",
                                 -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, ch.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *cha = (const char *)sqlite3_column_text(stmt, 0);
            result.push_back(std::string(cha));
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

std::vector<std::string>
ArrayContext::get_reverted_char_candidates_from_special(std::string keys) {
    std::vector<std::string> result;
    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(this->conn,
                                 "SELECT ch FROM main WHERE cat='2' AND keys=?",
                                 -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *ch = (const char *)sqlite3_column_text(stmt, 0);
            result.push_back(std::string(ch));
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

bool ArrayContext::input_key_is_not_special(std::string keys, std::string ch) {
    sqlite3_stmt *stmt;

    int retcode;
    std::vector<std::string> ret;
    bool result = false;

    retcode = sqlite3_prepare_v2(this->conn,
                                 "SELECT keys FROM main WHERE cat='2' AND ch=?",
                                 -1, &stmt, NULL);

    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, ch.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *cha = (const char *)sqlite3_column_text(stmt, 0);
            ret.push_back(std::string(cha));
        }
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    /*
     * We cannot find the word in special table,
     * so the word does not have special code.
     */
    if (ret.size() == 0) {
        return false;
    }

    if (ret[0].compare(keys) != 0) {
        result = true;
    }

    return result;
}

} // namespace Array
