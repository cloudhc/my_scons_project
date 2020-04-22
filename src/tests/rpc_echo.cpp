/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

#include "common/rpc_base.hpp"

class rpc_echo: public xa::rpc_base {
public:
    rpc_echo() : rpc_base("0.0.0.0", 10080)
    {
    }

    virtual ~rpc_echo()
    {
    }
    virtual bool serve(const Json::Value& req, Json::Value* res);
};

bool rpc_echo::serve(const Json::Value& req, Json::Value* res)
{
    (*res)["result"] = req;
    return true;
}

int main(int argc, char *argv[])
{
    rpc_echo echo;
    echo.start();
    echo.join();
}
