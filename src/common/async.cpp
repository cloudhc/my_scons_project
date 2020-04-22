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

#include <chrono>
#include <functional>
#include <future>

#include "async.hpp"
#include "logger.hpp"

namespace pca {

namespace async {

/**
 * Scanning for a given timeout.
 *
 * @param sec       a timeout.
 * @param checker   a checker functions reference.
 * @return true if success, otherwise false.
 */
bool scan_wait_for(int sec, std::function<bool()> checker_func)
{
    bool _running = true;
    auto scanning = [&]() -> void {
        while (_running) {
            if (checker_func()) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    };

    std::future<void> future = std::async(std::launch::async, scanning);
    std::future_status status = future.wait_for(std::chrono::seconds(sec));

    _running = false;
    if (std::future_status::timeout == status) {
#if !defined(UNIT_TEST)
        XA_LOGGER(fatal) << "timeout occurred because it was " << sec << " seconds.";
#endif
    }

    return std::future_status::ready == status ? true : false;
}

}  // namespace async

}  // namespace pca
