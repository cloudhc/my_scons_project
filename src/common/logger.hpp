/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            Boost Logger API documentation.
 */

/**
 * @file logger.hpp
 *
 * @brief      Xabyss's Boost Logger library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>

namespace pca {

namespace logger {

#define MAX_FILE_SIZE 25*1024*1024

// helper macro
std::string path_to_filename(std::string path);
#if defined(UNIT_TEST)
#define XA_LOGGER(sv) std::cerr
#else
#define XA_LOGGER(sv) BOOST_LOG_TRIVIAL(sv) \
        << '[' << pca::logger::path_to_filename(__FILE__) << ": " <<  __LINE__ << "] "
#endif

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace expr = boost::log::expressions;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void init(const char*, const char*);
void trace(std::string);
void debug(std::string);
void info(std::string);
void warning(std::string);
void error(std::string);
void fatal(std::string);
void do_output_swinfo(const char* module_name);

}  // namespace logger

}  // namespace pca
