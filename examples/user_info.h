// Copyright (c) 2021 Nomango

#include <jsonxx/json.hpp>

using namespace jsonxx;
using std::vector;
using std::string;

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

extern UserInfo QueryUser(int user_id);

// 绑定json
template <>
struct json_bind<UserRole>
{
	void to_json(json& j, const UserRole& v)
	{
		jsonxx::to_json(j["code"], v.code);
		jsonxx::to_json(j["permission_list"], v.permission_list);
	}

	void from_json(const json& j, UserRole& v)
	{
		jsonxx::from_json(j["code"], v.code);
		jsonxx::from_json(j["permission_list"], v.permission_list);
	}
};

template <>
struct json_bind<UserInfo>
{
	void to_json(json& j, const UserInfo& v)
	{
		jsonxx::to_json(j["user_id"], v.user_id);
		jsonxx::to_json(j["user_name"], v.user_name);
		jsonxx::to_json(j["role_list"], v.role_list);
	}

	void from_json(const json& j, UserInfo& v)
	{
		jsonxx::from_json(j["user_id"], v.user_id);
		jsonxx::from_json(j["user_name"], v.user_name);
		jsonxx::from_json(j["role_list"], v.role_list);
	}
};
