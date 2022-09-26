// Copyright (c) 2021 Nomango

#include "handler.h"

#include <iostream>

int main(int argc, char** argv)
{
    std::istringstream req("{\n    \"user_id\": 10001\n}\n");
    std::ostringstream resp;

    GetUserInfoHandler handler;
    handler.POST(req, resp);

    std::cout << "request: \n" << req.str() << std::endl;
    std::cout << "response: \n" << resp.str() << std::endl;
    return 0;
}
