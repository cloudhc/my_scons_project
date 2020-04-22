/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            Asyncnous Communication API documentation.
 */

/**
 * @file async.hpp
 *
 * @brief      Xabyss's Asyncnous Communication library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <functional>

namespace pca {

namespace async {

bool scan_wait_for(int sec, std::function<bool()> checker_func);

}  // namespace async

}  // namespace pca
