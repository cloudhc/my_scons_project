/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "common/logger.hpp"
#include "common/validation.hpp"

using pca::validation::validate_ipv4_address;
using pca::validation::validate_port_number;
using pca::validation::validate_netmask;

TEST_CASE("common_validation_test")
{
    SECTION("Checking validate_ipv4_address() function.") {
        SECTION("Best practice case") {
            REQUIRE(validate_ipv4_address("localhost"));
            REQUIRE(validate_ipv4_address("127.0.0.1"));
            REQUIRE(validate_ipv4_address("10.189.0.100"));
            REQUIRE(validate_ipv4_address("www.google.com"));
            REQUIRE(validate_ipv4_address("www.naver.com"));
        }
        SECTION("Worst practice case") {
            REQUIRE(!validate_ipv4_address("10.1.1"));
            REQUIRE(!validate_ipv4_address("www.22t.com"));
        }
    }

    SECTION("Checking validate_port_number() function.") {
        SECTION("Best practice case") {
            REQUIRE(validate_port_number(1));
            REQUIRE(validate_port_number(3306));
            REQUIRE(validate_port_number(65535));
        }
        SECTION("Worst practice case") {
            REQUIRE(!validate_port_number(-1));
            REQUIRE(!validate_port_number(65536));
        }
    }

    SECTION("Checking validate_netmask() function.") {
        SECTION("Best practice case") {
            REQUIRE(validate_netmask(1));
            REQUIRE(validate_netmask(8));
            REQUIRE(validate_netmask(16));
            REQUIRE(validate_netmask(24));
            REQUIRE(validate_netmask(32));
        }
        SECTION("Worst practice case") {
            REQUIRE(!validate_netmask(33));
            REQUIRE(!validate_netmask(64));
            REQUIRE(!validate_netmask(65535));
        }
    }
}
