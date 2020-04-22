/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            Xabyss's Commandline Parser API documentation.
 */

/**
 * @file cmdline.hpp
 *
 * @brief      Xabyss's Commandline Parser library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

namespace pca {

#define ENUM_BEGIN(t) \
    static struct option t ## _options [] = {

#define START_ENUM(n) \
    { "config", required_argument, NULL, 'c' }, \
    { "debug", 0, NULL, 'D' },                  \
    { "help", 0, NULL, 'H' },

#define TEST_ENUM(n) \
    { "test", 0, NULL, 't' },

#define END_ENUM(n) \
    { NULL, 0, NULL, 0 },

#define ENUM_END(t) \
    };

#define PARSER_BEGIN(t) \
    const char* config_filename = NULL;         \
    int option_idx;                             \
    int c;                                      \
                                                \
    while ((c = getopt_long(argc, argv, #t, long_options, &option_idx)) >= 0) { \
        switch (c) {

#define START_PARSER(n) \
    case 'c':                                   \
        config_filename = optarg;               \
        break;                                  \
    case 'D':                                   \
        debug_level++;                          \
        break;                                  \
    case 'H':                                   \
        show_help(argv[0]);                     \
        ::exit(0);

#define TEST_PARSER(n) \
    case 't':                                   \
        test_mode = true;                       \
        break;

#define END_PARSER(n) \
    default:                                    \
        logger::fatal("Use --help for usage");  \
        ::exit(1);                              \
    }

#define PARSER_END(t) \
    }                                           \
                                                \
    argc = (argc - optind + 1);                 \
    argv = &argv[optind - 1];                   \
                                                \
    if (config_filename != NULL && !parse_config(config_filename)) {                \
        logger::fatal(fmt::format("Parsing config {} failed", config_filename));    \
        ::exit(1);                                                                  \
    }

#define HELP_BASE() \
    printf("  -c, --config=FILE     Load config file\n");       \
    printf("  -D, --debug           Increase debug level\n");   \
    printf("  -H, --help            Show this message\n");

#define HELP_TEST() \
    printf("  -t, --test            Test mode\n");

#define MAKE_CLI_PARSER()   \
do {                        \
    ENUM_BEGIN(long)        \
        START_ENUM(3)       \
        END_ENUM(1)         \
    ENUM_END(long)          \
                            \
    PARSER_BEGIN("c:D?H")   \
        START_PARSER(3)     \
        END_PARSER(1)       \
    PARSER_END("c:D?H")     \
} while (0)

#define MAKE_CLI_PARSER_WITH_TMODE()  \
do {                        \
    ENUM_BEGIN(long)        \
        START_ENUM(3)       \
        TEST_ENUM(1)        \
        END_ENUM(1)         \
    ENUM_END(long)          \
                            \
    PARSER_BEGIN("c:D?Ht")  \
        START_PARSER(3)     \
        TEST_PARSER(1)      \
        END_PARSER(1)       \
    PARSER_END("c:D?Ht")    \
} while (0)

#define MAKE_CLI_HELP()     \
do {                        \
    HELP_BASE()             \
} while (0)

#define MAKE_CLI_HELP_WITH_TMODE()     \
do {                        \
    HELP_BASE()             \
    HELP_TEST()             \
} while (0)

}  // namespace pca
