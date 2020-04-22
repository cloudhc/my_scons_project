/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (c) 2014 Xabyss Inc. All rights reserved.
 */

/**
 * @mainpage  Main Page
 *
 *            Mini-HTTP Server API documentation.
 */

/**
 * @file rpc_base.cpp
 *
 * @brief      Xabyss's Mini-HTTP Server library source file.
 * @details    This header file must be included in any xabyss's probe applications.
 */

#include <vector>

#include "rpc_base.hpp"

#define RPC_PORT 10080
#define RPC_PATH "/rpc"

namespace xa {

struct rpc_base::rpc_handler {
    rpc_base *server;
    explicit rpc_handler(rpc_base *r)
    {
        server = r;
    }

    static bool is_valid_req(Json::Value const &req)
    {
        if (!req.isObject())
            return false;

        // check member list
        std::vector<std::string> members = req.getMemberNames();
        for (const auto& it : members) {
            if ((it != "jsonrpc") && (it != "method") && (it != "params")
                && (it != "id"))
                return false;
        }

        if ((!req.isMember("jsonrpc")) || (!req.isMember("method"))
            || (!req.isMember("id")))
            return false;

        if ((!req["jsonrpc"].isString()) || (req["jsonrpc"] != "2.0")
            || (!req["method"].isString()))
            return false;

        // optional "params" should be structure value
        if (req.isMember("params")) {
            if ((!req["params"].isArray()) && (!req["params"].isObject()))
                return false;
        }

        return true;
    }

    static void set_response_error(int code, std::string msg, Json::Value* res)
    {
        (*res)["error"] = Json::Value(Json::objectValue);
        (*res)["error"]["code"] = code;
        (*res)["error"]["message"] = msg;
        (*res)["id"] = Json::Value(Json::nullValue);
        (*res)["jsonrpc"] = "2.0";
    }

    static int http_status_from_error_code(int code)
    {
        switch (code) {
        case rpc_base::ERROR_METHOD_NOT_FOUND:
            return 404;
        case rpc_base::ERROR_INVALID_PARAMS:
        case rpc_base::ERROR_INTERNAL_ERROR:
            return 500;
        default:
            if (code >= rpc_base::ERROR_SERVER_ERROR_START
                    && code <= rpc_base::ERROR_SERVER_ERROR_END)
                return 500;
            break;
        }

        return 500;
    }

    int serve(const Json::Value& req, Json::Value* res)
    {
        int code = server->serve(req, res) ? 200
            : http_status_from_error_code((*res)["error"]["code"].asInt());

        (*res)["id"] = req["id"];
        (*res)["jsonrpc"] = "2.0";

        return code;
    }

    void operator()(http_server::request const &request, http_server::response &response)
    {
        std::string path = destination(request);
        if (path == "/test" && method(request) == "GET") {
            response = http_server::response::stock_reply(http_server::response::ok, "Testing 1,2,3");
            return;
        }

        if (path != "/rpc") {
            response = http_server::response::stock_reply(http_server::response::not_found, "not found");
            return;
        }

        if (method(request) == "OPTIONS") {
            response = http_server::response::stock_reply(http_server::response::ok, "");
            http_server::response_header header;

            header = { "Content-Type", "application/json" };
            response.headers.push_back(header);
            header = { "Allow", "GET,POST,OPTIONS" };
            response.headers.push_back(header);
            if (server->allow_cors()) {
                header = { "Access-Control-Allow-Origin", "*" };
                response.headers.push_back(header);
                header = { "Access-Control-Allow-Methods", "GET,POST,OPTIONS" };
                response.headers.push_back(header);

                for (const auto& h : request.headers) {
                    if (h.name == "Access-Control-Request-Headers") {
                        header = { "Access-Control-Allow-Headers", h.value };
                        response.headers.push_back(header);
                    }
                }
            }

            return;
        } else if (method(request) == "POST") {
            int code;
            Json::Value json_req, json_res;
            Json::Reader reader;
            Json::FastWriter writer;

            if (!reader.parse(body(request), json_req)) {
                set_response_error(rpc_base::ERROR_PARSE_ERROR, "Parse Error", &json_res);
                code = 500;
            } else if (json_req.isObject()) {
                if (is_valid_req(json_req)) {
                    code = serve(json_req, &json_res);
                } else {
                    set_response_error(rpc_base::ERROR_INVALID_REQUEST, "Invalid Request", &json_res);
                    code = 400;
                }
            } else if (json_req.isArray()) {
                code = 200;
                json_res = Json::Value(Json::arrayValue);
                for (const auto& item_req : json_req) {
                    Json::Value item_res(Json::objectValue);

                    if (is_valid_req(item_req)) {
                        int item_status = serve(item_req, &item_res);
                        if (item_status > code)
                            code = item_status;
                    } else {
                        set_response_error(rpc_base::ERROR_INVALID_REQUEST, "Invalid Request", &item_res);
                        code = 400;
                    }
                    json_res.append(item_res);
                }
            } else {
                set_response_error(rpc_base::ERROR_INVALID_REQUEST, "Invalid Request", &json_res);
                code = 400;
            }

            response = http_server::response::stock_reply(
                (http_server::response::status_type) code,
                writer.write(json_res));
            http_server::response_header header;

            header = { "Content-Type", "application/json" };
            response.headers.push_back(header);
            if (server->allow_cors()) {
                header = { "Access-Control-Allow-Origin", "*" };
                response.headers.push_back(header);
            }
        } else {
            response = http_server::response::stock_reply(http_server::response::bad_request, "bad request");
        }
    }

    void log(http_server::string_type const &info)
    {
        // ignored
    }
};

rpc_base::rpc_base(const std::string& address, unsigned short port, int nthreads)
    : listen_address_(address)
    , listen_port_(port)
    , nthreads_(nthreads)
{
}

rpc_base::~rpc_base()
{
}

void rpc_base::start()
{
    thread_ = std::thread(&rpc_base::run, this);
#ifdef DEBUG
    pthread_setname_np(thread_.native_handle(), "rpc");
#endif
}

void rpc_base::stop()
{
    server_->stop();
}

void rpc_base::join()
{
    thread_.join();
}

void rpc_base::run()
{
    rpc_handler handler(this);
    http_server::options options(handler);

    server_ = std::unique_ptr<http_server>(new http_server(
                options.address(listen_address_).port(std::to_string(listen_port_)).reuse_address(true)));

    std::thread services[nthreads_ - 1];
    if (nthreads_ >= 2) {
        for (std::thread& service : services) {
            service = std::thread(&http_server::run, server_.get());
        }
    }
    server_->run();  // run 1 in the main thread
    if (nthreads_ >= 2) {
        for (std::thread& service : services) {
            service.join();
        }
    }
}

bool rpc_base::serve_method_not_found(const Json::Value &params, Json::Value* res)
{
    (*res)["error"] = Json::Value(Json::objectValue);
    (*res)["error"]["code"] = ERROR_METHOD_NOT_FOUND;
    (*res)["error"]["message"] = "Invalid Request";

    return false;
}

bool rpc_base::serve_unimplemented(const Json::Value &params, Json::Value* res)
{
    (*res)["error"] = Json::Value(Json::objectValue);
    (*res)["error"]["code"] = ERROR_INTERNAL_ERROR;
    (*res)["error"]["message"] = "Not Implemented";

    return false;
}

}  // namespace xa
