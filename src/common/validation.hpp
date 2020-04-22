/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            Validation helper API documentation.
 */

/**
 * @file validation.hpp
 *
 * @brief      Xabyss's Validation helper library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <string>

namespace pca {

namespace validation {

bool validate_ipv4_address(const std::string& s);
bool validate_port_number(const uint32_t& port);
bool validate_netmask(const uint16_t& netmask);

}  // namespace validation

}  // namespace pca
