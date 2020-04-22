/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            MariaDB helper API documentation.
 */

/**
 * @file mariadb.hpp
 *
 * @brief      Xabyss's MariaDB helper library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <mysql.h>
#include <string>

#include <boost/exception/all.hpp>

namespace pca {

namespace mariadb {

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info;

typedef struct db_params_ {
    std::string db_scheme;
    std::string db_user;
    std::string db_pass;
    std::string db_host;
    uint16_t db_port;
    std::string db_name;
} db_params;

bool affected_rows_db(MYSQL& db, long expected);
bool connect_db(MYSQL& db, const std::string& uri);
void connect_db_with_except(MYSQL& db, const std::string& uri);
bool disconnect_db(MYSQL& db, MYSQL_RES* result = NULL);
int fetch_num_of_rows_db(MYSQL_RES* result);
int fetch_num_of_fields_db(MYSQL_RES* result);
MYSQL_RES* fetch_result_db_with_except(MYSQL& db, const unsigned idx = 0);
MYSQL_ROW fetch_row_db(MYSQL_RES* result);
bool free_result_db(MYSQL_RES* result);
void query_db_with_except(MYSQL& db, const char* query);
bool parse_database_uri(const std::string& uri, db_params* params);
using fn_body = std::function<void(MYSQL&, MYSQL_RES*)>;
bool query_helper(fn_body func, const std::string& uri);
bool save_alarm_to_db(const char type, const int code, const std::string& uri);

}  // namespace mariadb

}  // namespace pca
