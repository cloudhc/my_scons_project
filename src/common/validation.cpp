/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

/**
 * @mainpage  Main Page
 *
 *            Validation helper API documentation.
 */

/**
 * @file validation.cpp
 *
 * @brief      Xabyss's Validation helper library source file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <regex>

#include "validation.hpp"

namespace pca {

namespace validation {

/**
 * Check IP address.
 *
 * @param s         domainname or ipv4 address.
 * @return true if validation check is success, false otherwise.
 */
bool validate_ipv4_address(const std::string& s) {
    static const std::regex re("([-_0-9-\\.]+)");
    std::smatch m;

    if (std::regex_match(s, m, re)) {  // prase ipv4 address
        static const std::regex re("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");

        if (std::regex_match(s, m, re)) {
            struct in_addr in_addr;
            if (inet_aton(s.c_str(), &in_addr) != 0)
                return true;
        }

        return false;
    } else {  // parse domainname
        struct hostent *host_entry;
        host_entry = gethostbyname(s.c_str());

        return (host_entry != NULL);
    }
}

/**
 * Check TCP/UDP port number.
 *
 * @param port      A tcp port number (range: 1 ~ 65535).
 * @return true if validation check is success, false otherwise.
 */
bool validate_port_number(const uint32_t& port)
{
    return (port >= 0 && port <= 65535);
}

/**
 * Check IPv4 netmask.
 *
 * @param port      A netmask (range: 1 ~ 32).
 * @return true if validation check is success, false otherwise.
 */
bool validate_netmask(const uint16_t& netmask)
{
    return (netmask > 0 && netmask <= 32);
}

}  // namespace validation

}  // namespace pca
