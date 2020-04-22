/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

/**
 * @mainpage  Main Page
 *
 *            MariaDB helper API documentation.
 */

/**
 * @file mariadb.cpp
 *
 * @brief      Xabyss's MariaDB helper library source file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <netdb.h>
#include <exception>
#include <iostream>
#include <regex>

#include <boost/asio.hpp>

#include "logger.hpp"
#include "mariadb.hpp"
#include "validation.hpp"

namespace pca {

namespace mariadb {

/**
 *  @brief A dbinitexcept exception class.
 *  @ingroup exceptions
 *
 *  The mariadb library throws objects of this class if an error ocuured during initializing mysql_init().
 */
struct dbinitexcept : public boost::exception, public std::exception {
    const char *what() const noexcept { return "Database init failed "; }
};

/**
 *  @brief A dbinvalidexcept exception class.
 *  @ingroup exceptions
 *
 *  The mariadb library throws objects of this class if a error ocurred during parsing given database URI string.
 */
struct dbinvalidargexcept : public boost::exception, public std::exception {
    const char *what() const noexcept { return "Database Argument invlid "; }
};

/**
 *  @brief A dbconnexcept exception class.
 *  @ingroup exceptions
 *
 *  The mariadb library throws objects of this class if a error ocurred during connecting to database server.
 */
struct dbconnexcept : public boost::exception, public std::exception {
    const char *what() const noexcept { return "Database connect failed "; }
};

/**
 *  @brief A dbqueryexcept exception class.
 *  @ingroup exceptions
 *
 *  The mariadb library throws objects of this class if a error ocurred during executing given SQL statement.
 */
struct dbqueryexcept : public boost::exception, public std::exception {
    const char *what() const noexcept { return "Database query failed "; }
};

/**
 *  @brief A dbresultexcept exception class.
 *  @ingroup exceptions
 *
 *  The mariadb library throws objects of this class if a error ocurred during fetching a result after execute SQL statement.
 */
struct dbresultexcept : public boost::exception, public std::exception {
    const char *what() const noexcept { return "Database result failed "; }
};

/**
 *  @brief A dbdisconnexcept exception class.
 *  @ingroup exceptions
 *
 *  The mariadb library throws objects of this class if a error ocurred during disconnecting to database server.
 */
struct dbdisconnexcept : public boost::exception, public std::exception {
    const char *what() const noexcept { return "Dabase disconnect failed "; }
};

/**
 * Check if the expected number of result is equal.
 *
 * @param db        a mysql handle.
 * @param expected  an expected value.
 * @return ture if the expected number of results is equal, false otherwise.
 */
bool affected_rows_db(MYSQL& db, long expected)
{
    long affected_rows = mysql_affected_rows(&db);

    return (affected_rows == expected) ? true : false;
}

/**
 * Establishes a connection to a database server.
 *
 * @param db        a mysql handle, which was previously allocated by mysql_init().
 * @param uri       the URI string.
 * @return true on success or false if an error occurred.
 */
bool connect_db(MYSQL& db, const std::string& uri)
{
    if (mysql_init(&db) == NULL) {
        return false;
    }

    db_params params;
    if (!parse_database_uri(uri, &params)) {
        return false;
    }

    if (mysql_real_connect(&db,
                           params.db_host.c_str(),
                           params.db_user.c_str(),
                           params.db_pass.c_str(),
                           params.db_name.c_str(),
                           params.db_port,
                           NULL, CLIENT_MULTI_STATEMENTS) == NULL) {
        return false;
    }

    return true;
}

/**
 * Establishes a connection to a database server, throw an exception if connection failed.
 *
 * @param db        a mysql handle, which was previously allocated by mysql_init().
 * @param uri       the URI string.
 * @throws an exception of dbinitexcept, dbinvalidargexcept, dbconnexcept.
 */
void connect_db_with_except(MYSQL& db, const std::string& uri)
{
    try {
        if (mysql_init(&db) == NULL)
            BOOST_THROW_EXCEPTION(dbinitexcept{});

        db_params params;
        if (!parse_database_uri(uri, &params))
            BOOST_THROW_EXCEPTION(dbinvalidargexcept{});

        if (mysql_real_connect(&db,
                               params.db_host.c_str(),
                               params.db_user.c_str(),
                               params.db_pass.c_str(),
                               params.db_name.c_str(),
                               params.db_port,
                               NULL, CLIENT_MULTI_STATEMENTS) == NULL)
            BOOST_THROW_EXCEPTION(dbconnexcept{});
    } catch (dbinitexcept& e) {
        std::stringstream ss;
        ss << e.what() << mysql_error(&db);
        e << errmsg_info{ss.str()};
        throw;
    } catch (dbinvalidargexcept& e) {
        std::stringstream ss;
        ss << e.what() << "\"" << uri.c_str() << "\"(mysql://USER[:PASS]@HOST[:PORT]/DB)";
        e << errmsg_info{ss.str()};
        throw;
    } catch (dbconnexcept& e) {
        std::stringstream ss;
        ss << e.what() << mysql_error(&db);
        e << errmsg_info{ss.str()};
        throw;
    }
}

/**
 * Fetch result to execute an SQL statement, throw exception if an error occurred.
 *
 * @param db        a mysql handle.
 * @param idx       an idx for debugging.
 * @throws an exception of dbresultexcept.
 */
MYSQL_RES* fetch_result_db_with_except(MYSQL& db, const unsigned idx)
{
    try {
        MYSQL_RES* result = mysql_store_result(&db);

        if (result == NULL)
            BOOST_THROW_EXCEPTION(dbresultexcept{});

        return result;
    } catch (dbresultexcept& e) {
        std::stringstream ss;
        ss << e.what() << ", idx : " << idx;
        e << errmsg_info{ss.str()};

        mysql_close(&db);

        throw;
    }
}

/**
 * Disconnet a connection to a database server.
 *
 * @param db        a mysql handle.
 * @param result    a pointer of MYSQL_RES structure for release resource.
 * @return always successful
 */
bool disconnect_db(MYSQL& db, MYSQL_RES* result)
{
    if (result)
        mysql_free_result(result);

    mysql_close(&db);

    return true;
}

/**
 * Fetch number of result to execute an SQL statement.
 *
 * @param result     a pointer of MYSQL_RES structure.
 * @return an number of results
 */
int fetch_num_of_rows_db(MYSQL_RES* result)
{
    return mysql_num_rows(result);
}

/**
 * Fetch number of field to execute an SQL statement.
 *
 * @param result     a pointer of MYSQL_RES structure.
 * @return an number of field
 */
int fetch_num_of_fields_db(MYSQL_RES* result)
{
    return mysql_num_fields(result);
}

/**
 * Fetch record of result to execute an SQL statement.
 *
 * @param result    a pointer of MYSQL_RES structure.
 * @return a MYSQL_ROW structure for the next row, or NULL
 */
MYSQL_ROW fetch_row_db(MYSQL_RES* result)
{
    return mysql_fetch_row(result);
}

/**
 * Release resource of result to execute an SQL statement.
 *
 * @param result    a pointer of MYSQL_RES structure for release resource.
 * @return always successful
 */
bool free_result_db(MYSQL_RES* result)
{
    mysql_free_result(result);

    return true;
}

/**
 * Parse database URI string format.
 *
 * @param uri       the URI string.
 * @param scheme    the protocol for connecting database server. (output)
 * @param User      the username. (output)
 * @param pass      the password. (output)
 * @param host      a host name or an IP address. (output)
 * @param port      the port number to connect to the server. (output)
 * @param dbname    the default database to be used when performing queries. (output)
 * @return true if URI string parsiing success, false otherwise.
 */
bool parse_database_uri(const std::string& uri, db_params* params)
{
    using pca::validation::validate_ipv4_address;
    using pca::validation::validate_port_number;

    std::smatch match;
    std::regex re("^([a-z]+)://(\\w+)(?::(\\w+))?@([-_0-9a-z-\\.]+)(?::([0-9]+))?/([a-zA-Z_]+)$");

    bool matched = std::regex_match(uri, match, re);
    if (!matched)
        return false;

    auto it = match.begin();
    ++it;
    (*params).db_scheme = *it;
    ++it;
    (*params).db_user = *it;
    ++it;
    (*params).db_pass = *it;
    ++it;
    (*params).db_host = *it;
    if (!validate_ipv4_address(*it)) {
        return false;
    }
    ++it;
    if (*it == "") {
        if ((*params).db_scheme == "mysql")
            (*params).db_port = 3306;
        else
            return false;
    } else {
        int value = std::stoi(*it);
        if (!validate_port_number(value))
            return false;
        (*params).db_port = value;
    }
    ++it;
    (*params).db_name = *it;

    return true;
}

/**
 * Execute an SQL statement, throw exception if an error occurred.
 *
 * @param db        a mysql handle.
 * @param query     an SQL statement to execute.
 * @throws an exception of dbqueryexcept.
 */
void query_db_with_except(MYSQL& db, const char* query)
{
    try {
        if (mysql_query(&db, query) != 0)
            BOOST_THROW_EXCEPTION(dbqueryexcept{});
    } catch(dbqueryexcept& e) {
        std::stringstream ss;
        ss << e.what() << mysql_error(&db) << " (query : " << query << ")";
        e << errmsg_info{ss.str()};

        mysql_close(&db);

        throw;
    }
}

/**
 * Helper function to execute an SQL statement.
 *
 * @param func          a function table of database queries.
 * @param database_uri  a URI string.
 * @return true on success or false if an error occurred.
 */
bool query_helper(fn_body func, const std::string& uri)
{
    MYSQL db;
    MYSQL_RES* result = NULL;

    try {
        connect_db_with_except(db, uri);

        func(db, result);

        disconnect_db(db, result);
    } catch (std::bad_function_call& bfc) {
        logger::fatal(bfc.what());

        disconnect_db(db, result);

        return false;
    } catch (std::invalid_argument& iva) {
        logger::error(iva.what());

        disconnect_db(db, result);

        return false;
    } catch (boost::exception& e) {
        logger::error(*boost::get_error_info<errmsg_info>(e));
        // std::cerr << boost::diagnostic_information(e);

        return false;
    } catch (...) {
        logger::fatal("unknown exception occurred, please check your code.");

        return false;
    }

    return true;
}

bool save_alarm_to_db(const char type, const int code, const std::string& uri) {
    MYSQL db;

    try {
        connect_db_with_except(db, uri);

        char sql_query[1024];
        snprintf(sql_query, sizeof(sql_query),
                "INSERT INTO apca_dev.event_list values(NULL,'%c',%d,sysdate(),NULL);", type, code);

        query_db_with_except(db, sql_query);

        bool isInsertDone = affected_rows_db(db, 1L);
        if (!isInsertDone)
            throw std::invalid_argument("Database insert be failed, please check insert value");

        disconnect_db(db);

        return true;
    } catch(boost::exception& e) {
        XA_LOGGER(error) << *boost::get_error_info<errmsg_info>(e);

        return false;
    } catch(...) {
        XA_LOGGER(error) << "Unknown exception occurred, please check your code.";

        return false;
    }
}

}  // namespace mariadb

}  // namespace pca
