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
#include <sstream>
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

    json::value v = json::object{
        { "test", 1 },
        { "array", json::array{ 1, 2 } },
    };

    User u = { 1001, "中文" };

    const auto str =
        json::dump<wchar_t>(u, {
                                   json::serializer_type<wchar_t>::with_indent(2),
                                   json::serializer_type<wchar_t>::with_unicode_escaping(false),
                                   json::serializer_type<wchar_t>::with_source_encoding<encoding::auto_utf>(),
                                   json::serializer_type<wchar_t>::with_target_encoding<encoding::auto_utf>(),
                               });
    std::wcout << str << std::endl;

    u = json::parse(L"{\"id\": 1002,\"name\":\"Jack中文\"}",
                    {
                        json::parser_type<wchar_t>::with_error_handler(nullptr),
                        json::parser_type<wchar_t>::with_source_encoding<encoding::auto_utf>(),
                        json::parser_type<wchar_t>::with_target_encoding<encoding::auto_utf>(),
                    });

    json::dump(std::wcout, u);
    std::wcout << std::endl;
    return 0;
}
