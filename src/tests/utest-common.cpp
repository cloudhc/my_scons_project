/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#define CATCH_CONFIG_MAIN
#include <vector>

#include <boost/algorithm/string/join.hpp>

#include "catch2/catch.hpp"
#include "common/logger.hpp"
#include "common/mariadb.hpp"
#include "common/utils.hpp"

using pca::mariadb::connect_db_with_except;
using pca::mariadb::db_params;
using pca::mariadb::disconnect_db;
using pca::mariadb::parse_database_uri;

TEST_CASE("common_machine_test")
{
    SUCCEED();
}

TEST_CASE("common_mariadb_test")
{
    SECTION("Checking normal URI-1") {
        std::string normal_uri1 = "mysql://apca_user:apca1213@localhost:3306/apca_dev";
        db_params params;

        REQUIRE(parse_database_uri(normal_uri1, &params));
        REQUIRE(params.db_scheme == "mysql");
        REQUIRE(params.db_user == "apca_user");
        REQUIRE(params.db_pass == "apca1213");
        REQUIRE(params.db_host == "localhost");
        REQUIRE(params.db_port == 3306);
        REQUIRE(params.db_name == "apca_dev");
    }

    SECTION("Checking normal URI-2") {
        std::string normal_uri2 = "mysql://apca_user:apca1213@127.0.0.1:3306/apca_dev";
        db_params params;

        REQUIRE(parse_database_uri(normal_uri2, &params));
        REQUIRE(params.db_scheme == "mysql");
        REQUIRE(params.db_user == "apca_user");
        REQUIRE(params.db_pass == "apca1213");
        REQUIRE(params.db_host == "127.0.0.1");
        REQUIRE(params.db_port == 3306);
        REQUIRE(params.db_name == "apca_dev");
    }

    SECTION("Checking normal URI-3") {
        std::string normal_uri3 = "mysql://apca_user:apca1213@localhost/apca_dev";
        db_params params;

        REQUIRE(parse_database_uri(normal_uri3, &params));
        REQUIRE(params.db_scheme == "mysql");
        REQUIRE(params.db_user == "apca_user");
        REQUIRE(params.db_pass == "apca1213");
        REQUIRE(params.db_host == "localhost");
        REQUIRE(params.db_port == 3306);  // HACK: get default port using mysql scheme
        REQUIRE(params.db_name == "apca_dev");
    }

    SECTION("Checking abnormal URI-1") {
        std::string bad_uri1 = "mysql://apca_user:apca1213@localhost:65536/apca_dev";
        db_params params;

        REQUIRE(!parse_database_uri(bad_uri1, &params));
        REQUIRE(params.db_scheme == "mysql");
        REQUIRE(params.db_user == "apca_user");
        REQUIRE(params.db_pass == "apca1213");
        REQUIRE(params.db_host == "localhost");
        REQUIRE(params.db_port == 0);
        REQUIRE(params.db_name == "");
    }

    SECTION("Checking abnormal URI-2") {
        std::string bad_uri2 = "mysql://apca_user:apca1213@1.1.1:3406/apca_dev";
        db_params params;
        params.db_port = 0;  // HACK(hyunchul.lee@xabyss.com) : waste value clean

        REQUIRE(!parse_database_uri(bad_uri2, &params));
        REQUIRE(params.db_scheme == "mysql");
        REQUIRE(params.db_user == "apca_user");
        REQUIRE(params.db_pass == "apca1213");
        REQUIRE(params.db_host == "1.1.1");
        REQUIRE(params.db_port == 0);
        REQUIRE(params.db_name == "");
    }

    SECTION("Checking abnormal URI-3") {
        std::string bad_uri3 = "mysql://apca_user:apca1213@255.255.255:3406/apca_dev";
        db_params params;
        params.db_port = 0;  // HACK(hyunchul.lee@xabyss.com) : waste value clean

        REQUIRE(!parse_database_uri(bad_uri3, &params));
        REQUIRE(params.db_scheme == "mysql");
        REQUIRE(params.db_user == "apca_user");
        REQUIRE(params.db_pass == "apca1213");
        REQUIRE(params.db_host == "255.255.255");
        REQUIRE(params.db_port == 0);
        REQUIRE(params.db_name == "");
    }
}

TEST_CASE("exception_handler_test")
{
    std::string bad_uri = "mysql://apca_user:apca1213@127.0.0.1:65536/apca_dev";
    {
        MYSQL db;
        MYSQL_RES* result = NULL;

        REQUIRE_THROWS(connect_db_with_except(db, bad_uri));
        REQUIRE(disconnect_db(db, result));
    }
}

TEST_CASE("common_utils_test")
{
    using pca::utils::is_equal;
    using pca::utils::is_equal_count;
    using pca::utils::is_nocase_equal;
    using pca::utils::is_nocase_equal_count;
    using pca::utils::ends_with;

    SECTION("Checking is_equal() function") {
        SECTION("Best practice case") {
            REQUIRE(is_equal("ABCDE12345!@#$%", "ABCDE12345!@#$%"));
            REQUIRE(is_equal("abcde12345!@#$%", "abcde12345!@#$%"));
            REQUIRE(is_equal("", ""));
        }
        SECTION("Worst practice case") {
            REQUIRE(!is_equal("ABCDE12345!@#$%", "abcde12345!@#$%"));
            REQUIRE(!is_equal("abcde12345!@#$%", "ABCDE12345!@#$%"));
            REQUIRE(!is_equal("ABCDE12345!@#$%", ""));
            REQUIRE(!is_equal("ABCDE12345!@#$%", nullptr));
            REQUIRE(!is_equal("", "ABCDE12345!@#$%"));
            REQUIRE(!is_equal(nullptr, "ABCDE12345!@#$%"));
            REQUIRE(!is_equal(nullptr, nullptr));
        }
    }

    SECTION("Checking is_equal_count() function") {
        SECTION("Best practice case") {
            REQUIRE(is_equal_count("ABCDE12345!@#$%", "ABCDE12345!@#$%", 15));
            REQUIRE(is_equal_count("abcde12345!@#$%", "abcde12345!@#$%", 15));
            REQUIRE(is_equal_count("ABCDE12345!@#$%", "ABCDE12345!@#$%", 25));
            REQUIRE(is_equal_count("abcde12345!@#$%", "abcde12345!@#$%", 25));
            REQUIRE(is_equal_count("", "", 1));
        }
        SECTION("Worst practice case") {
            REQUIRE(!is_equal_count("ABCDE12345!@#$%", "abcde12345!@#$%", 15));
            REQUIRE(!is_equal_count("abcde12345!@#$%", "ABCDE12345!@#$%", 15));
            REQUIRE(!is_equal_count("ABCDE12345!@#$%", "", 1));
            REQUIRE(!is_equal_count("ABCDE12345!@#$%", nullptr, 0));
            REQUIRE(!is_equal_count("", "ABCDE12345!@#$%", 1));
            REQUIRE(!is_equal_count(nullptr, "ABCDE12345!@#$%", 0));
            REQUIRE(!is_equal_count(nullptr, nullptr, 0));
        }
    }

    SECTION("Checking is_nocase_equal() function") {
        SECTION("Best practice case") {
            REQUIRE(is_nocase_equal("ABCDE12345!@#$%", "ABCDE12345!@#$%"));
            REQUIRE(is_nocase_equal("abcde12345!@#$%", "abcde12345!@#$%"));
            REQUIRE(is_nocase_equal("ABCDE12345!@#$%", "abcde12345!@#$%"));
            REQUIRE(is_nocase_equal("abcde12345!@#$%", "ABCDE12345!@#$%"));
            REQUIRE(is_nocase_equal("", ""));
        }
        SECTION("Worst practice case") {
            REQUIRE(!is_nocase_equal("ABCDE12345!@#$%", ""));
            REQUIRE(!is_nocase_equal("ABCDE12345!@#$%", nullptr));
            REQUIRE(!is_nocase_equal("", "ABCDE12345!@#$%"));
            REQUIRE(!is_nocase_equal(nullptr, "ABCDE12345!@#$%"));
            REQUIRE(!is_nocase_equal(nullptr, nullptr));
        }
    }

    SECTION("Checking is_nocase_equal_count() function") {
        SECTION("Best practice case") {
            REQUIRE(is_nocase_equal_count("ABCDE12345!@#$%", "ABCDE12345!@#$%", 15));
            REQUIRE(is_nocase_equal_count("abcde12345!@#$%", "abcde12345!@#$%", 15));
            REQUIRE(is_nocase_equal_count("ABCDE12345!@#$%", "abcde12345!@#$%", 15));
            REQUIRE(is_nocase_equal_count("abcde12345!@#$%", "ABCDE12345!@#$%", 15));
            REQUIRE(is_nocase_equal_count("ABCDE12345!@#$%", "abcde12345!@#$%", 25));
            REQUIRE(is_nocase_equal_count("abcde12345!@#$%", "ABCDE12345!@#$%", 25));
            REQUIRE(is_nocase_equal_count("", "", 1));
        }
        SECTION("Worst practice case") {
            REQUIRE(!is_nocase_equal_count("ABCDE12345!@#$%", "", 15));
            REQUIRE(!is_nocase_equal_count("ABCDE12345!@#$%", nullptr, 15));
            REQUIRE(!is_nocase_equal_count("", "ABCDE12345!@#$%", 1));
            REQUIRE(!is_nocase_equal_count(nullptr, "ABCDE12345!@#$%", 0));
            REQUIRE(!is_nocase_equal_count(nullptr, nullptr, 0));
        }
    }

    SECTION("Checking end_with() function") {
        SECTION("Best practice Case") {
            REQUIRE(ends_with("abcdef.ghi", ".ghi"));
            REQUIRE(ends_with("abcdef.txt", ".txt"));
            REQUIRE(ends_with("abcdef.jpeg", ".jpeg"));
            REQUIRE(ends_with("a.bcdefg", ".bcdefg"));
            REQUIRE(ends_with("abcdef.ghi", "ghi"));
            REQUIRE(ends_with("abcdef.txt", "txt"));
            REQUIRE(ends_with("abcdef.jpeg", "jpeg"));
            REQUIRE(ends_with("a.bcdefg", "bcdefg"));
            REQUIRE(ends_with("abcdef.ghi", "hi"));
            REQUIRE(ends_with("abcdef.txt", "xt"));
            REQUIRE(ends_with("abcdef.jpeg", "peg"));
            REQUIRE(ends_with("a.bcdefg", "cdefg"));
            REQUIRE(ends_with(".ghi", ".ghi"));
            REQUIRE(ends_with(".txt", ".txt"));
            REQUIRE(ends_with(".jpeg", ".jpeg"));
            REQUIRE(ends_with("abcdef.ghi", ""));
            REQUIRE(ends_with("abcdef.txt", ""));
            REQUIRE(ends_with("abcdef.jpeg", ""));
            REQUIRE(ends_with("a.bcdefg", ""));
            REQUIRE(ends_with("abcdef.ghi", ""));
            REQUIRE(ends_with("abcdef.txt", ""));
            REQUIRE(ends_with("abcdef.jpeg", ""));
            REQUIRE(ends_with("a.bcdefg", ""));
            REQUIRE(ends_with("", ""));
        }

        SECTION("Worst practice Case") {
            REQUIRE(!ends_with("abcdef.ghi", nullptr));
            REQUIRE(!ends_with("abcdef.txt", nullptr));
            REQUIRE(!ends_with("abcdef.jpeg", nullptr));
            REQUIRE(!ends_with("a.bcdefg", nullptr));
            REQUIRE(!ends_with("bcdefg", ".bcdefg"));
            REQUIRE(!ends_with("ghi", ".ghi"));
            REQUIRE(!ends_with("txt", ".txt"));
            REQUIRE(!ends_with("jpeg", ".jpeg"));
            REQUIRE(!ends_with("bcdefg", ".bcdefg"));
            REQUIRE(!ends_with("", ".ghi"));
            REQUIRE(!ends_with(nullptr, ".jpeg"));
            REQUIRE(!ends_with(nullptr, nullptr));
        }
    }

    SECTION("to_pretty_datetime test") {
        using pca::utils::to_pretty_datetime;

        SECTION("case-1") {
            using Catch::Matchers::Equals;

            const char* input = R"({
   "id" : 1,
   "jsonrpc" : "2.0",
   "method" : "search_sessions",
   "params" : {
      "exclude" : [],
      "from" : {
         "nsec" : 0,
         "sec" : 1574067066
      },
      "include" : [],
      "limit" : 100,
      "offset" : 0,
      "to" : {
         "nsec" : 0,
         "sec" : 1574153466
      }
   }
})";

            const char* expected = R"({
   "id" : 1,
   "jsonrpc" : "2.0",
   "method" : "search_sessions",
   "params" : {
      "exclude" : [],
      "from" : {
         "nsec" : 0,
         "sec" : 1574067066 (2019-11-18 17:51:06)
      },
      "include" : [],
      "limit" : 100,
      "offset" : 0,
      "to" : {
         "nsec" : 0,
         "sec" : 1574153466 (2019-11-19 17:51:06)
      }
   }
})";
            REQUIRE_THAT(to_pretty_datetime(input), Equals(expected));
        }
    }

    SECTION("string join", "std::vector<std::string>") {
        using Catch::Matchers::Equals;

        SECTION("boost join test") {
            std::vector<std::string> v{"1", "2", "3", "4", "5"};

            std::string joined = boost::algorithm::join(v, "/");

            REQUIRE_THAT(joined, Equals("1/2/3/4/5"));
        }

        SECTION("boost join test with std::initialize_list") {
            std::initializer_list<std::string> v = {"1", "2", "3", "4", "5"};

            std::string joined = boost::algorithm::join(v, "/");

            REQUIRE_THAT(joined, Equals("1/2/3/4/5"));
        }

        SECTION("boost join test with std::initialize_list") {
            std::initializer_list<std::string> v = {"1"};

            std::string joined = boost::algorithm::join(v, "/");

            REQUIRE_THAT(joined, Equals("1"));
        }

        SECTION("to_join library test") {
            std::initializer_list<std::string> v = {"1", "2", "3"};

            REQUIRE_THAT(pca::utils::to_join(v, "/"), Equals("1/2/3"));
        }
    }
}
