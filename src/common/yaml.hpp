/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            Yaml's Parser API documentation.
 */

/**
 * @file yaml.hpp
 *
 * @brief      Xabyss's Yaml's Parser library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <yaml.h>
#include <exception>
#include <functional>
#include <map>
#include <list>
#include <string>

namespace xa {

namespace yaml {

#define DEBUG_COMMON_ENUM() \
{                                                                                                       \
    "debug",                                                                                            \
    [](const std::string& key, xa::yaml::node& value) -> bool                                           \
    {                                                                                                   \
        if (key == "level") {                                                                           \
            /* Note: the command line option (-D) precedes this in debug.level in config file */        \
            unsigned val = value.as_integer();                                                          \
            if (val > debug_level)                                                                      \
                debug_level = val;                                                                      \
        }                                                                                               \
                                                                                                        \
        return true;                                                                                    \
    }                                                                                                   \
}

#define DATA_SEARCH_ENUM() \
{                                                                                                       \
    "data",                                                                                             \
    [](const std::string& key, xa::yaml::node& value) -> bool                                           \
    {                                                                                                   \
        if (key == "path") {                                                                            \
            output_file_path = value.as_string();                                                       \
        } else if (key == "session-heartbeat-timeout") {                                                \
            session_heartbeat_timeout_sec = value.as_integer();                                         \
        } else if (key == "session-search-timeout") {                                                   \
            session_search_timeout_sec = value.as_integer();                                            \
        }                                                                                               \
                                                                                                        \
        return true;                                                                                    \
    }                                                                                                   \
}

#define DOWNLOAD_SEARCH_ENUM() \
{                                                                                                       \
    "download",                                                                                         \
    [](const std::string& key, xa::yaml::node& value) -> bool                                           \
    {                                                                                                   \
        if (key == "database") {                                                                        \
            download_database_uri = value.as_string();                                                  \
        } else if (key == "path") {                                                                     \
            download_path = value.as_string();                                                          \
        }                                                                                               \
                                                                                                        \
        return true;                                                                                    \
    }                                                                                                   \
}


class node;

using fn_body = std::function<bool(const std::string&, xa::yaml::node&)>;

namespace errc
{
    enum errc_t
    {
        success = 0,
        file_not_found,
        yaml_parser_initialize_fail,
        type_mismatch,
        invalid_argument,
        not_supported,
        unknown_except
    };
}  // namespace errc

struct error_code {
    errc::errc_t val_;
    std::string message_;

    error_code(): val_(errc::success), message_("success") {}

    errc::errc_t value() const noexcept
    {
        return val_;
    }

    std::string& message() noexcept
    {
        return message_;
    }
};

class exception_base : public std::exception {
public:
    explicit exception_base(const char* s) : msg(s)
    {
    }

    virtual const char* what()
    {
        return msg.c_str();
    }

    std::string msg;
};

class type_mismatch : public exception_base {
public:
    explicit type_mismatch(const char* s) : exception_base(s)
    {
    }
};

class yaml_parse_fail : public exception_base {
public:
    explicit yaml_parse_fail(const char* s) : exception_base(s)
    {
    }
};

class node {
public:
    enum node_type {
        TYPE_EMPTY,
        TYPE_SCALAR,
        TYPE_SEQUENCE,
        TYPE_MAPPING
    };

    node_type type;
    std::string scalar;
    std::list<node> sequence;
    std::map<std::string, node> mapping;

    error_code errc_;

    node() : type(TYPE_EMPTY)
    {
    }

    explicit node(node_type t) : type(t)
    {
    }

    std::string& as_scalar()
    {
        if (type != TYPE_SCALAR)
            throw type_mismatch("Not a scalar node");
        return scalar;
    }

    std::list<node>& as_sequence()
    {
        if (type != TYPE_SEQUENCE)
            throw type_mismatch("Not a sequence node");
        return sequence;
    }

    std::map<std::string, node>& as_mapping()
    {
        if (type != TYPE_MAPPING)
            throw type_mismatch("Not a mapping node");
        return mapping;
    }

    std::string& as_string()
    {
        return as_scalar();
    }

    uint32_t as_integer()
    {
        return std::stoi(as_scalar());
    }

    bool as_boolean()
    {
        return (as_scalar() == "true");
    }

    bool load_file(const char* filename);
    bool load_string(const char* str, size_t n);

    // helper method
    bool load_settings(const char* filename, std::map<std::string, fn_body>& yaml_settings);

private:
    bool load_file_with_fmap(const char* filename, std::map<std::string, fn_body>& parsers);
    bool load_mapping(yaml_parser_t* parser);
    bool load_node(yaml_parser_t* parser, yaml_event_t* event);
    bool load_sequence(yaml_parser_t* parser);
    bool load_top(yaml_parser_t* parser);
};

}  // namespace yaml

}  // namespace xa
