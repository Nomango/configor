// Copyright (c) 2021 Nomango

// #include "handler.h"

// #include <iostream>

// int main(int argc, char** argv)
// {
//     std::istringstream req("{\n    \"user_id\": 10001\n}\n");
//     std::ostringstream resp;

//     GetUserInfoHandler handler;
//     handler.POST(req, resp);

//     std::cout << "request: \n" << req.str() << std::endl;
//     std::cout << "response: \n" << resp.str() << std::endl;
//     return 0;
// }

#include <configor/json.hpp>
#include <iostream>
#include <locale>
#include <string>

using namespace configor;

struct User
{
    int         id;
    std::string name;

    CONFIGOR_BIND(json::value, User, REQUIRED(id), REQUIRED(name));
};

int main(int argc, char** argv)
{
    std::wcout.imbue(std::locale(""));

    json::value v = {
        { "test", 1 },
        { "array中文", { 1, 2 } },
    };

    User u = { 1001, "John" };

    const auto str = json::dump(v, {
                                       json::serializer<>::with_indent(2),
                                       json::serializer<>::with_escaping_unicode(false),
                                       json::serializer<>::with_source_encoding<encoding::auto_utf>(),
                                       json::serializer<>::with_target_encoding<encoding::auto_utf>(),
                                   });
    std::cout << str << std::endl;

    u = json::parse("{\"id\": 1002,\"name\":\"Jack\"}", {
                                                            json::parser::with_error_handler(nullptr),
                                                            json::parser::with_source_encoding<encoding::auto_utf>(),
                                                            json::parser::with_target_encoding<encoding::auto_utf>(),
                                                        });

    json::dump(std::wcout, u);
    std::wcout << std::endl;
    return 0;
}
