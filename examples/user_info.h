// Copyright (c) 2021 Nomango

#include <jsonxx/json.hpp>
#include <memory>

using namespace jsonxx;
using std::string;
using std::vector;

// 用户角色
struct UserRole
{
    // 角色编号
    int code;
    // 权限列表
    vector<string> permission_list;
};

// 用户信息
struct UserInfo
{
    // 用户id
    int user_id;
    // 用户名
    string user_name;
    // 角色列表
    vector<UserRole> role_list;
};

extern std::shared_ptr<UserInfo> QueryUser(int user_id);

// 绑定json
template <>
struct jsonxx::json_bind<UserRole>
{
    static void to_json(json& j, const UserRole& v)
    {
        j = { { "code", v.code }, { "permission_list", v.permission_list } };
    }

    static void from_json(const json& j, UserRole& v)
    {
        v.code            = j["code"];
        v.permission_list = j["permission_list"];
    }
};

template <>
struct jsonxx::json_bind<UserInfo>
{
    static void to_json(json& j, const UserInfo& v)
    {
        j = { { "user_id", v.user_id }, { "user_name", v.user_name }, { "role_list", v.role_list } };
    }

    static void from_json(const json& j, UserInfo& v)
    {
        v.user_id   = j["user_id"];
        v.user_name = (std::string)j["user_name"];
        v.role_list = j["role_list"];
    }
};
