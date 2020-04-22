/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

/**
 * @mainpage  Main Page
 *
 *            Mini-HTTP Server API documentation.
 */

/**
 * @file rpc_base.hpp
 *
 * @brief      Xabyss's Mini-HTTP Server library header file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <json/json.h>
#include <sys/socket.h>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include <boost/network/protocol/http/server.hpp>

namespace xa {

class rpc_base {
public:
    // http://www.jsonrpc.org/specification#error_object
    constexpr static const int ERROR_PARSE_ERROR = -32700;
    constexpr static const int ERROR_INVALID_REQUEST = -32600;
    constexpr static const int ERROR_METHOD_NOT_FOUND = -32601;
    constexpr static const int ERROR_INVALID_PARAMS = -32602;
    constexpr static const int ERROR_INTERNAL_ERROR = -32603;
    constexpr static const int ERROR_SERVER_ERROR_START = -32000;
    constexpr static const int ERROR_SERVER_ERROR_END = -32099;

    virtual bool serve(const Json::Value &req, Json::Value* res) = 0;

    void start();
    void stop();
    void join();
    void run();

    void set_allow_cors(bool enable);

protected:
    rpc_base(const std::string& address, unsigned short port, int nthreads = 1);
    virtual ~rpc_base();

    bool allow_cors() const;
    bool serve_build_info(const Json::Value& params, Json::Value* res);
    bool serve_method_not_found(const Json::Value& params, Json::Value* res);
    bool serve_unimplemented(const Json::Value& params, Json::Value* res);

private:
    std::thread thread_;
    std::string listen_address_;
    unsigned short listen_port_;
    int nthreads_;

    bool allow_cors_ = false;

    struct rpc_handler;
    typedef boost::network::http::server<rpc_handler> http_server;
    std::unique_ptr<http_server> server_;
};

inline void rpc_base::set_allow_cors(bool enable)
{
    allow_cors_ = enable;
}

inline bool rpc_base::allow_cors() const
{
    return allow_cors_;
}

}  // namespace xa
