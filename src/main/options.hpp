/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once
#include <string>
#include <utility>
#include <vector>

#include "mysql.h"
#include "common/yaml.hpp"

namespace pca {

class options {
public:
    static std::vector<std::string> args;

    static std::string path_prefix;

    // debug
    static unsigned debug_level;

    // settings
    static bool settings_enabled;
    static std::string settings_database_uri;

    // control
    static bool control_enabled;
    static std::string control_listen_address;
    static uint16_t control_listen_port;
    static bool control_allow_cors;

public:
    static bool parse_cmdline(int argc, char *argv[]);
    static bool parse_config(const std::string& s);
    static bool load_settings_from_database();

protected:
    static bool parse_config(const char* filename);
    static std::string get_address_port_string(std::string ip, std::string netmask, std::string port);

private:
    static bool load_dbsettings_all(MYSQL& db);
    // static std::list<int> get_changed_list();
    static bool parse_config(const xa::yaml::node& config);
};

}  // namespace pca
