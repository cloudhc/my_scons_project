/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#pragma once

#include <string>

#include "common/rpc_base.hpp"

namespace pca {

class rpc : public xa::rpc_base {
public:
    rpc(const std::string& address, unsigned short port);
    virtual bool serve(const Json::Value &req, Json::Value* res);

private:
    // interactive
    bool serve_ping(const Json::Value& params, Json::Value* res);
};

}  // namespace pca
