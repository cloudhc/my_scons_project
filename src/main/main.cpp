/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */
#include <chrono>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>
#include <execinfo.h>

#include "common/async.hpp"
#include "common/logger.hpp"
#include "common/mariadb.hpp"

#include "fmt/format.h"

#include "options.hpp"
#include "rpc.hpp"

using fmt::literals::operator""_format;

namespace pca {

namespace main {

volatile static bool is_running = true;

static void do_terminate(int sig)
{
    logger::trace("terminate by signal = {}"_format(sig));

    is_running = false;

    exit(1);
}

static void do_stacktrace(int sig)
{
    void *stack[100];
    size_t stacksize;

    // get void*'s for all entries on the stack
    stacksize = backtrace(stack, 100);

    // print out all the frames to stderr
    logger::fatal("Error: signal {:d}: stacksize = {:d}"_format(sig, stacksize));

    backtrace_symbols_fd(stack, stacksize, STDERR_FILENO);

    exit(1);
}

static void do_output_features()
{
    auto message = [](const char* msg, const bool value) {
        BOOST_LOG_TRIVIAL(info) << msg << (value ? "YES" : "NO");
    };

    BOOST_LOG_TRIVIAL(info) << " CONTROL ----------------------";
    message("          DAEMON : ", options::control_enabled);
    message("      ALLOW CORS : ", options::control_allow_cors);
}

void capture_init(int argc, char* argv[])
{
    logger::init("/opt/xabyss/css/log", "main");
    logger::do_output_swinfo("xa-main");

    signal(SIGSEGV, do_stacktrace);
    signal(SIGTERM, do_terminate);
    setenv("TZ", "UTC", 1);

    if (!options::parse_cmdline(argc, argv)) {
        logger::fatal("Invalid cmdline argument, Please check it");
        ::exit(1);
    }

    do_output_features();
}

void capture_main_loop()
{
    rpc rpc(options::control_listen_address, options::control_listen_port);
    if (options::control_enabled) {
        rpc.set_allow_cors(options::control_allow_cors);
        rpc.start();
    }

    printf("Hello, world!\n");

    while(is_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    printf("Exit!\n");

    if (options::control_enabled) {
        rpc.stop();
        rpc.join();
    }
}

}  // namespace main

}  // namespace pca

int main(int argc, char *argv[])
{
    pca::main::capture_init(argc, argv);
    pca::main::capture_main_loop();

    return 0;
}
