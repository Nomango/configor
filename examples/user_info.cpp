// Copyright (c) 2021 Nomango

#include "user_info.h"

std::shared_ptr<UserInfo> QueryUser(int user_id)
{
    if (user_id != 10001)
    {
        return nullptr;
    }
    auto ptr = std::make_shared<UserInfo>(UserInfo{
        10001,
        "John",
        {
            UserRole{ 100, { "READ" } },
            UserRole{ 101, { "DELETE", "MODIFY" } },
        },
    });
    return ptr;
}
