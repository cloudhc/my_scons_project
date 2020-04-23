/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

/**
 * @mainpage  Main Page
 *
 *            Logger API documentation.
 */

/**
 * @file logger.cpp
 *
 * @brief      Xabyss's logger library source file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include "common/logger.hpp"

namespace pca {

namespace logger {

/**
 * Initialize Boost Logger API.
 *
 * @param path      a directory path to be created log-file.
 * @param prefix    a log file name.
 */
void init(const char* path, const char* prefix)
{
#if !defined(UNIT_TEST)
    std::stringstream ss;
    ss << path << "/" << prefix << ".log";

    logging::add_file_log(
        keywords::file_name = ss.str().c_str(),
        keywords::rotation_size = MAX_FILE_SIZE,
        keywords::open_mode = std::ios_base::app,
        keywords::auto_flush = true,
        keywords::format = (
            expr::stream
                << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                << ": <" << logging::trivial::severity << "> "
                << expr::smessage));

    logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::trace);

    logging::add_common_attributes();
#endif
}

/**
 * Output a message of trace level.
 *
 * @param tra       a message to output.
 */
void trace(std::string tra)
{
#if !defined(UNIT_TEST)
    BOOST_LOG_TRIVIAL(trace) << tra;
#endif
}

/**
 * Output a message of debug level.
 *
 * @param deb       a message to output.
 */
void debug(std::string deb)
{
#if !defined(UNIT_TEST)
    BOOST_LOG_TRIVIAL(debug) << deb;
#endif
}

/**
 * Output a message of information level.
 *
 * @param inf       a message to output.
 */
void info(std::string inf)
{
#if !defined(UNIT_TEST)
    BOOST_LOG_TRIVIAL(info) << inf;
#endif
}

/**
 * Output a message of warning level.
 *
 * @param war       a message to output.
 */
void warning(std::string war)
{
#if !defined(UNIT_TEST)
    BOOST_LOG_TRIVIAL(warning) << war;
#endif
}

/**
 * Output a message of error level.
 *
 * @param err       a message to output.
 */
void error(std::string err)
{
#if !defined(UNIT_TEST)
    BOOST_LOG_TRIVIAL(error) << err;
#endif
}

/**
 * Output a message of fatal level.
 *
 * @param fat       a message to output.
 */
void fatal(std::string fat)
{
#if !defined(UNIT_TEST)
    BOOST_LOG_TRIVIAL(fatal) << fat;
#endif
}

/**
 * Get filename at full pathname.
 *
 * @param fat       a message to output.
 * @return a file name.
 */
std::string path_to_filename(std::string path)
{
    return path.substr(path.find_last_of("/\\")+1);
}

/**
 * Output S/W information to log file.
 *
 * @param module_name   a module name to write to the log file.
 */
void do_output_swinfo(const char* module_name)
{
    auto get_build_number = []() -> std::string {
        std::string build_number = "Main";

        build_number += " (COMMIT-ID: 0000000)";

        return build_number;
    };

    BOOST_LOG_TRIVIAL(info) << "started boost log by " << module_name << "...";
    BOOST_LOG_TRIVIAL(info) << "---------------------------------------------";
    BOOST_LOG_TRIVIAL(info) << "      BUILD TIME : " << std::string(__DATE__) << ", " << std::string(__TIME__);
    BOOST_LOG_TRIVIAL(info) << "    BUILD NUMBER : " << get_build_number();
    BOOST_LOG_TRIVIAL(info) << "---------------------------------------------";
}

}  // namespace logger

}  // namespace pca
