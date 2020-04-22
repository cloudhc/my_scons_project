/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

/**
 * @mainpage  Main Page
 *
 *            YAML API documentation.
 */

/**
 * @file yaml.cpp
 *
 * @brief      Xabyss's YAML library source file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <stdio.h>
#include <errno.h>

#include "fmt/format.h"

#include "logger.hpp"
#include "yaml.hpp"

namespace xa {

using fmt::literals::operator""_format;
using fmt::format;

namespace yaml {

/**
 * Parse a yaml data by given string.
 *
 * @param s         a given string of yaml data.
 * @param n         the length.
 * @throws an error if yaml_parser_initialize() is failed.
 * @return true on success, false otherwise.
 */
bool node::load_string(const char* s, size_t n)
{
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser))
        throw yaml_parse_fail("Can't initialize YAML parser");

    yaml_parser_set_input_string(&parser, reinterpret_cast<const unsigned char*>(s), n);
    bool result = load_top(&parser);
    yaml_parser_delete(&parser);

    return result;
}

/**
 * Parse a yaml data by given file.
 *
 * @param filename  a given yaml filename.
 * @throws an error if open() is failed or yaml_parser_initialize() is failed.
 * @return true on success, false otherwise.
 */
bool node::load_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        auto s = fmt::format("Can't open config {0}: {1}", filename, strerror(errno));
        throw yaml_parse_fail(s.c_str());
    }

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        fclose(file);
        throw yaml_parse_fail("Can't initialize YAML parser");
    }

    yaml_parser_set_input_file(&parser, file);
    bool result = load_top(&parser);
    yaml_parser_delete(&parser);
    fclose(file);

    return result;
}

/**
 * An helper function to process YAML parser.
 *
 * @param parser    a given yaml parser structure.
 * @throws an error ocurred during parsing YAML node.
 * @return true on success, false otherwise.
 */
bool node::load_top(yaml_parser_t* parser)
{
    bool result = true;

    while (true) {
        yaml_event_t event;

        if (!yaml_parser_parse(parser, &event)) {
            auto s = fmt::format("Parser error {}", parser->error);
            throw yaml_parse_fail(s.c_str());
        }

        if (event.type == YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type == YAML_SEQUENCE_START_EVENT ||
            event.type == YAML_MAPPING_START_EVENT) {
            result = load_node(parser, &event);
            yaml_event_delete(&event);
            break;
        }

        yaml_event_delete(&event);
    }

    return result;
}

/**
 * An helper function to process YAML node.
 *
 * @param parser    a given yaml parser structure.
 * @param event     a given yaml-event structure.
 * @throws an error ocurred during parsing YAML node.
 * @return true on success, false otherwise.
 */
bool node::load_node(yaml_parser_t* parser, yaml_event_t* event)
{
    if (event->type == YAML_SCALAR_EVENT) {
        type = TYPE_SCALAR;
        scalar.assign((const char*) event->data.scalar.value, event->data.scalar.length);
        return true;
    } else if (event->type == YAML_MAPPING_START_EVENT) {
        type = TYPE_MAPPING;
        return load_mapping(parser);
    } else if (event->type == YAML_SEQUENCE_START_EVENT) {
        type = TYPE_SEQUENCE;
        return load_sequence(parser);
    } else {
        auto s = fmt::format("Unexpected node type {}", event->type);
        throw yaml_parse_fail(s.c_str());
    }
}

/**
 * An helper function to process YAML node key.
 *
 * @param parser    a given yaml parser structure.
 * @throws an error ocurred during parsing YAML node.
 * @return true on success, false otherwise.
 */
bool node::load_mapping(yaml_parser_t* parser)
{
    while (true) {
        yaml_event_t event;

        if (!yaml_parser_parse(parser, &event)) {
            auto s = fmt::format("Parser error {}", parser->error);
            throw yaml_parse_fail(s.c_str());
        }

        if (event.type == YAML_MAPPING_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        std::string key((const char*) event.data.scalar.value, event.data.scalar.length);
        yaml_event_delete(&event);
        auto result = mapping.emplace(key, node{TYPE_EMPTY});
        if (!result.second) {
            auto s = fmt::format("Duplicated key {}", key);
            throw yaml_parse_fail(s.c_str());
        }

        if (!yaml_parser_parse(parser, &event)) {
            auto s = fmt::format("Parser error {}", parser->error);
            throw yaml_parse_fail(s.c_str());
        }

        (*result.first).second.load_node(parser, &event);

        yaml_event_delete(&event);
    }
    return true;
}

/**
 * An helper function to process YAML sequence node.
 *
 * @param parser    a given yaml parser structure.
 * @throws an error ocurred during parsing YAML node.
 * @return true on success, false otherwise.
 */
bool node::load_sequence(yaml_parser_t* parser)
{
    while (true) {
        yaml_event_t event;

        if (!yaml_parser_parse(parser, &event)) {
            auto s = fmt::format("Parser error {}", parser->error);
            throw yaml_parse_fail(s.c_str());
        }

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        sequence.emplace_back(TYPE_EMPTY);
        auto& back = sequence.back();

        if (!back.load_node(parser, &event)) {
            yaml_event_delete(&event);

            return false;
        }

        yaml_event_delete(&event);
    }
    return true;
}

/**
 * An helper function to process YAML metadata.
 *
 * @param filename  a given yaml filename.
 * @param parsers   a map of metadata to parse.
 * @throws an error ocurred during parsing.
 * @return true on success, false otherwise.
 */
bool node::load_file_with_fmap(const char* filename, std::map<std::string, fn_body>& parsers)
{
    try {
        load_file(filename);

        auto& mapping = as_mapping();
        for (auto& it : mapping) {
            const std::string& mkey = it.first;
            xa::yaml::node& mvalue = it.second;

            if (mvalue.type == node::TYPE_EMPTY) {
                auto s = format("Value is empty on {}", mkey);
                throw yaml_parse_fail(s.c_str());
            } else if (mvalue.type == node::TYPE_SCALAR) {
                continue;
            } else if (mvalue.type == node::TYPE_SEQUENCE) {
                const std::string& key = it.first;
                xa::yaml::node& value = it.second;

                if (!parsers[mkey](key, value)) {
                    auto s = format("parser failed : mkey {0}, key {1} with node type {2}", mkey, key, value.type);
                    throw yaml_parse_fail(s.c_str());
                }
            } else if (mvalue.type == node::TYPE_MAPPING) {
                auto& mapping = mvalue.as_mapping();
                for (auto& it : mapping) {
                    // validation check
                    if (it.second.type == node::TYPE_EMPTY) {
                        continue;
                    }
                    const std::string& key = it.first;
                    xa::yaml::node& value = it.second;

                    if (!parsers[mkey](key, value)) {
                        auto s = format("parser failed : mkey {0}, key {1} with node type {2}", mkey, key, value.type);
                        throw yaml_parse_fail(s.c_str());
                    }
                }
            } else {
                auto s = format("Node type is Unknown ({0}, {1})", mkey, mvalue.type);
                throw yaml_parse_fail(s.c_str());
            }
        }
        errc_.val_ = errc::success;
        errc_.message_ = "success";

        return true;
    } catch (xa::yaml::yaml_parse_fail& e) {
        errc_.val_ = errc::yaml_parser_initialize_fail;
        errc_.message_ = e.what();

        return false;
    } catch (xa::yaml::type_mismatch& e) {
        errc_.val_  = errc::type_mismatch;
        errc_.message_ = "Type mismatch. => {}"_format(e.what());

        return false;
    } catch (std::invalid_argument& e) {
        errc_.val_ = errc::invalid_argument;
        errc_.message_ = "There is error when change string to number. => {}"_format(e.what());

        return false;
    } catch(std::bad_function_call& e) {
        errc_.val_ = errc::not_supported;
        errc_.message_ = "There is no parser function by key. => {}"_format(e.what());

        return false;
    } catch(...) {
        errc_.val_ = errc::unknown_except;
        errc_.message_ = "Unkonwn excpetion occured, please check your code.";

        return false;
    }
}

/**
 * An helper function to process YAML file and result of parsing.
 *
 * @param filename      a given yaml filename.
 * @param yaml_settings a map of parseing data.
 * @throws an error ocurred during parsing YAML node.
 * @return true on success, false otherwise.
 */
bool node::load_settings(const char* filename, std::map<std::string, fn_body>& yaml_settings)
{
    bool result = load_file_with_fmap(filename, yaml_settings);

#if !defined(UNIT_TEST)
    pca::logger::debug("{0} loading result is {1} cause {2}"_format(
        filename,
        (errc_.value() == xa::yaml::errc::success ? "success" : "fail"),
        errc_.message()));
#endif

    if (errc_.value() == xa::yaml::errc::yaml_parser_initialize_fail)
        return false;

    return result;
}

}  // namespace yaml

}  // namespace xa
