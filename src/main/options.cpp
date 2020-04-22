/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <map>
#include <tuple>

#include "fmt/format.h"

#include <boost/asio.hpp>

#include "common/async.hpp"
#include "common/cmdline.hpp"
#include "common/logger.hpp"
#include "common/mariadb.hpp"
#include "common/utils.hpp"
#include "common/validation.hpp"

#include "options.hpp"

using fmt::literals::operator""_format;
using pca::async::scan_wait_for;
using pca::mariadb::connect_db;
using pca::mariadb::connect_db_with_except;
using pca::mariadb::disconnect_db;
using pca::mariadb::fetch_num_of_rows_db;
using pca::mariadb::fetch_num_of_fields_db;
using pca::mariadb::fetch_row_db;
using pca::mariadb::fetch_result_db_with_except;
using pca::mariadb::free_result_db;
using pca::mariadb::query_db_with_except;
using pca::utils::is_equal;
using pca::utils::is_nocase_equal;
using pca::validation::validate_port_number;

namespace pca {

static void show_help(const char *progname);

std::vector<std::string> options::args;

std::string options::path_prefix = ".";
unsigned options::debug_level = 0;

bool options::control_enabled = false;
std::string options::control_listen_address = "127.0.0.1";
uint16_t options::control_listen_port = 10081;
bool options::control_allow_cors = false;

bool options::settings_enabled = false;
std::string options::settings_database_uri;

bool options::parse_cmdline(int argc, char *argv[])
{
    MAKE_CLI_PARSER();

    if (settings_enabled) {
        load_settings_from_database();
    }

    return true;
}

bool options::parse_config(const char* filename)
{
    static std::map<std::string, xa::yaml::fn_body> yaml_settings = {
        DEBUG_COMMON_ENUM(),
        {
            "control",
            [](const std::string& key, xa::yaml::node& value) -> bool
            {
                if (key == "enabled") {
                    control_enabled = value.as_boolean();
                } else if (key == "listen-address") {
                    control_listen_address = value.as_string();
                } else if (key == "listen-port") {
                    control_listen_port = static_cast<uint16_t>(value.as_integer());
                    if (!validate_port_number(control_listen_port)) {
                        logger::error("invalid listen-port: {:+16d}"_format(control_listen_port));

                        return false;
                    }
                } else if (key == "allow-cors") {
                    control_allow_cors = value.as_boolean();
                }

                return true;
            }
        },
        {
            "paths",
            [](const std::string& key, xa::yaml::node& value) -> bool
            {
                if (key == "prefix") {
                    path_prefix = value.as_string();
                }

                return true;
            }
        },
        {
            "settings",
            [](const std::string& key, xa::yaml::node& value) -> bool
            {
                if (key == "enabled") {
                    settings_enabled = value.as_boolean();
                } else if (key == "database") {
                    settings_database_uri = value.as_string();
                }

                return true;
            }
        }
    };

    xa::yaml::node config;

    return config.load_settings(filename, yaml_settings);
}

bool options::parse_config(const std::string& s)
{
    xa::yaml::node config;

    if (!config.load_string(s.c_str(), s.size())) {
        logger::fatal("Can't load config from string");

        return false;
    }

    return parse_config(config);
}

bool options::parse_config(const xa::yaml::node& config)
{
    return true;
}

//
// Load settings from helper program pipe
//
bool options::load_settings_from_database()
{
    if (!settings_enabled)
        return false;

    MYSQL db;

    for (unsigned retry_count = 0; retry_count < 5; retry_count++) {
        if (!connect_db(db, options::settings_database_uri)) {
            logger::debug("db_connect failed: {}"_format(mysql_error(&db)));

            sleep(1);
            continue;
        }

        if (!load_dbsettings_all(db)) {
            logger::debug("load_dbsettings_all failed: {}"_format(mysql_error(&db)));

            disconnect_db(db);
            sleep(1);
            continue;
        }

        return true;
    }

    return false;
}

std::string options::get_address_port_string(std::string ip,  std::string netmask, std::string port)
{
    std::stringstream ss;

    // FIXME: ANY -> any
    std::transform(ip.begin(), ip.end(), ip.begin(), ::tolower);
    std::transform(netmask.begin(), netmask.end(), netmask.begin(), ::tolower);
    std::transform(port.begin(), port.end(), port.begin(), ::tolower);

    if (ip != "any")
        ss << ip;
    if (netmask != "any") {
        int n = 0;
        while (n <= 32) {
            uint32_t mask;
            char maskstr[32];
            mask = 0xFFFFFFFF - ((1 << n) - 1);
            snprintf(maskstr, sizeof(maskstr), "%d.%d.%d.%d",
                     (mask >> 24) & 0xFF, (mask >> 16) & 0xFF,
                     (mask >> 8) & 0xFF, (mask >> 0) & 0xFF);
            if (maskstr == netmask) {
                if (n != 0) {
                    ss << "/" << (32 - n);
                    break;
                }
            }
            n += 1;
        }
        if (n > 32)
            return "";
    }
    if (port != "any") {
        ss << ":" << port;
    }
    return ss.str();
}

bool options::load_dbsettings_all(MYSQL& db)
{
    enum db_field { sql_stmt, parser_func };

    using fn_body = std::function<void(MYSQL_RES*)>;
    static std::vector<std::tuple<const char*, fn_body>> db_settings = {
    };

    try {
        // setup time_zone
        query_db_with_except(db, "SET time_zone = \"+0:00\";");

        for (auto& setting : db_settings) {
            query_db_with_except(db, std::get<sql_stmt>(setting));

            MYSQL_RES* result = fetch_result_db_with_except(db);
            std::get<parser_func>(setting)(result);

            free_result_db(result);
        }

        return true;
    } catch(std::invalid_argument &iae) {
        XA_LOGGER(error) << iae.what();

        disconnect_db(db);

        return false;
    } catch(boost::exception& e) {
        XA_LOGGER(error) << "query be failed = [" << mysql_error(&db) << "]";

        return false;
    } catch(...) {
        XA_LOGGER(error) << "Unknown exception occurred, please check your code.";

        return false;
    }
}

static void show_help(const char *progname)
{
    printf("Usage: %s [OPTIONS...]\n\n", progname);
    printf("Run CAPTURE\n\n");

    MAKE_CLI_HELP();
}

}  // namespace pca
