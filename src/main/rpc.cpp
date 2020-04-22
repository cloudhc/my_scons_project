/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#include "rpc.hpp"
#include "options.hpp"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

namespace pca {

rpc::rpc(const std::string& address, unsigned short port)
    : rpc_base(address, port)
{
}

bool rpc::serve(const Json::Value& req, Json::Value* res)
{
    std::string method = req["method"].asString();

    if ("ping" == method) {
        return serve_ping(req["params"], res);
    } else {
        return serve_method_not_found(req["params"], res);
    }

    return true;
}

bool rpc::serve_ping(const Json::Value& params, Json::Value* res)
{
    (*res)["result"] = Json::Value(Json::objectValue);
    (*res)["result"]["message"] = "OK";
    (*res)["result"]["params"] = params;

    return true;
}

}  // namespace pca
