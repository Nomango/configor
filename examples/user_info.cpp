// Copyright (c) 2021 Nomango

#include "user_info.h"

UserInfo QueryUser(int user_id)
{
	if (user_id != 10001)
	{
		return UserInfo{};
	}
	return UserInfo{
		10001,
		"John",
		{
			{100, {"READ"}},
			{101, {"DELETE", "MODIFY"}},
		},
	};
}
