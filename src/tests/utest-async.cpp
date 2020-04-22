/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "common/async.hpp"
#include "common/logger.hpp"

using pca::async::scan_wait_for;

TEST_CASE("common_async_test")
{
    SECTION("Checking scan_wait_for function.") {
        uint32_t i = 0;
        auto test_func = [&]() -> bool {
            return (++i == 3);
        };

        SECTION("Best practice case") {
            i = 0;
            REQUIRE(scan_wait_for(5, test_func));
            REQUIRE(i == 3);
            i = 0;
            REQUIRE(!scan_wait_for(2, test_func));
            REQUIRE(i == 2);
        }
    }
}
