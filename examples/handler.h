// Copyright (c) 2021 Nomango

#include "user_info.h"

#include <iomanip>
#include <sstream>

using std::istringstream;
using std::ostringstream;

struct Request
{
    int user_id;
};

struct Response
{
    std::shared_ptr<UserInfo> user_info;
};

template <>
struct jsonxx::json_bind<Request>
{
    static void to_json(json& j, const Request& v)
    {
        j = { { "user_id", v.user_id } };
    }

    static void from_json(const json& j, Request& v)
    {
        v.user_id = j["user_id"];
    }
};

template <>
struct jsonxx::json_bind<Response>
{
    static void to_json(json& j, const Response& v)
    {
        j = { { "user_info", v.user_info } };
    }

    static void from_json(const json& j, Response& v)
    {
        v.user_info = j["user_info"];
    }
};

// 获取用户信息接口
class GetUserInfoHandler
{
public:
    // POST请求
    void POST(istringstream& req, ostringstream& resp)
    {
        // 解析请求，可以直接反序列化到 Request 结构体中
        Request req_body;
        req >> json_wrap(req_body);

        // 读取用户信息
        auto user_info = QueryUser(req_body.user_id);

        // 响应请求，可以直接序列化到输出流中
        Response resp_body = { user_info };
        resp << std::setw(4) << json_wrap(resp_body);
    }
};
