/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "common/validation.hpp"

using pca::validation::validate_ipv4_address;

TEST_CASE("Benchmarking") {
    SECTION("validation benchmark test") {
        // now let's benchmark
        BENCHMARK("validate_ipv4_address(\"localhost\")") {
            return validate_ipv4_address("localhost");
        };

        BENCHMARK("validate_ipv4_address(\"127.0.0.1\")") {
            return validate_ipv4_address("127.0.0.1");
        };

        BENCHMARK("validate_ipv4_address(\"10.189.0.1\")") {
            return validate_ipv4_address("10.189.0.1");
        };

        BENCHMARK("validate_ipv4_address(\"www.google.com\")") {
            return validate_ipv4_address("www.google.com");
        };

        BENCHMARK("validate_ipv4_address(\"www.naver.com\")") {
            return validate_ipv4_address("www.naver.com");
        };

        BENCHMARK("not validate_ipv4_address(\"10.1.1\")") {
            return !validate_ipv4_address("www.naver.com");
        };

        BENCHMARK("not validate_ipv4_address(\"www.22t.com\")") {
            return !validate_ipv4_address("www.22.com");
        };
    }
}
