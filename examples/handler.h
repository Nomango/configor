// Copyright (c) 2021 Nomango

#include "user_info.h"

#include <iomanip>
#include <sstream>

struct Request
{
    int user_id;

    CONFIGOR_BIND_ALL_REQUIRED(json, Request, user_id);
};

struct Response
{
    std::shared_ptr<UserInfo> user_info;

    CONFIGOR_BIND_ALL_REQUIRED(json, Response, user_info);
};

// 获取用户信息接口
class GetUserInfoHandler
{
public:
    // POST请求
    void POST(std::istringstream& req, std::ostringstream& resp)
    {
        // 解析请求，可以直接反序列化到 Request 结构体中
        Request req_body;
        req >> json::wrap(req_body);

        // 读取用户信息
        auto user_info = QueryUser(req_body.user_id);

        // 响应请求，可以直接序列化到输出流中
        Response resp_body = { user_info };
        resp << std::setw(4) << json::wrap(resp_body);
    }
};
