// Copyright (c) 2021 Nomango

#include <configor/json.hpp>
#include <memory>

using namespace configor;
using std::string;
using std::vector;

// 用户角色
struct UserRole
{
    // 角色编号
    int code;
    // 权限列表
    vector<string> permission_list;

    CONFIGOR_BIND_ALL_REQUIRED(json, UserRole, code, permission_list);
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

    CONFIGOR_BIND_ALL_REQUIRED(json, UserInfo, user_id, user_name, role_list);
};

extern std::shared_ptr<UserInfo> QueryUser(int user_id);
