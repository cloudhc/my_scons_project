/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            C-Style string API documentation.
 */

/**
 * @file utils.hpp
 *
 * @brief      Xabyss's String library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <sys/time.h>
#include <string.h>

#include <string>
#include <regex>

#include <boost/algorithm/string/join.hpp>

namespace pca {

namespace utils {

inline bool is_equal_base(const char* s1, const char* s2, int (*compare)(const char*, const char*)) {
    if (s1 == nullptr || s2 == nullptr)
        return false;

    return compare(s1, s2) == 0;
}

inline bool is_equal_base_with_count(const char* s1, const char* s2, size_t count,
        int (*compare)(const char*, const char*, size_t)) {
    if (s1 == nullptr || s2 == nullptr)
        return false;

    return compare(s1, s2, count) == 0;
}

/**
 * Compares the C string s1 to the C string s2.
 *
 * @param s1        C string to be compared.
 * @param s2        C string to be compared.
 * @return true if the two strings are equal, false otherwise.
 */
static inline bool is_equal(const char* s1, const char* s2)
{
    return is_equal_base(s1, s2, strcmp);
}

/**
 * Compares the C string s1 to the C string s2 until count characters.
 *
 * @param s1        C string to be compared.
 * @param s2        C string to be compared.
 * @param count     Maximum number of characters to compare.
 *                  size_t is an unsigned integral type.
 * @return true if the two strings are equal, false otherwise.
 */
static inline bool is_equal_count(const char* s1, const char* s2, size_t count)
{
    return is_equal_base_with_count(s1, s2, count, strncmp);
}

/**
 * Compares the C string s1 to the C string s2 without sensitivity to case.
 *
 * @param s1        C string to be compared.
 * @param s2        C string to be compared.
 * @return true if the two strings are equal, false otherwise.
 */
static inline bool is_nocase_equal(const char* s1, const char* s2)
{
    return is_equal_base(s1, s2, strcasecmp);
}

/**
 * Compares the C string s1 to the C string s2 without sensitivity to case until count characters.
 *
 * @param s1        C string to be compared.
 * @param s2        C string to be compared.
 * @param count     Maximum number of characters to compare.
 *                  size_t is an unsigned integral type.
 * @return true if the two strings are equal, false otherwise.
 */
static inline bool is_nocase_equal_count(const char* s1, const char* s2, size_t count)
{
    return is_equal_base_with_count(s1, s2, count, strncasecmp);
}

/**
 * Compares the C string s ends with the C string ss.
 *
 * @param s         C string to be compared.
 * @param ss        C string to check if the string ends with.
 * @return true if the fisrt string(s) ends with the specified value(ss), otherwise false.
 */
static inline bool ends_with(const char* s, const char* ss)
{
    if (s == nullptr || ss == nullptr)
        return false;

    const size_t s_len = strlen(s);
    const size_t ss_len = strlen(ss);

    if (ss_len == 0)
        return true;

    return (s_len >= ss_len && is_equal_count(s + s_len - ss_len, ss, ss_len));
}

/**
 * Join list of string.
 *
 * @param strings   list of string.
 * @param separator delimiter string.
 * @return Joined string.
 */
static inline std::string to_join(const std::initializer_list<std::string>& strings, const char* separator)
{
    return boost::algorithm::join(strings, separator);
}

/**
 * Convert timestamp to datetime format.
 *
 * @param s         C string with timestamp only format.
 * @return converted string with datetime format.
 */
static inline std::string to_pretty_datetime(std::string s)
{
    auto to_str = [](const time_t raw_time) -> std::string {
        struct tm dt;
        char buffer[30];

        localtime_r(&raw_time, &dt);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &dt);

        return std::string(buffer);
    };
    std::smatch m;
    std::regex re("\"sec\" : [0-9]{10}");

    std::stringstream ss;
    while (std::regex_search(s, m, re)) {
        ss << m.prefix().str();
        std::string sec = m[0];
        ss << m[0] << " (" << to_str(std::stol(sec.erase(0, sec.find(" : ") + 3))) << ")";
        s = m.suffix().str();
    }
    ss << s;

    return ss.str();
}

}  // namespace utils

}  // namespace pca
