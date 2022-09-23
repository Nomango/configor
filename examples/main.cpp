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

using namespace configor;

int main(int argc, char** argv)
{
    json::value v = {
        { "test", 1 },
        { "array中文", { 1, 2 } },
    };

    const auto str = json::dump(v, {
                                       json::serializer::with_indent(2),
                                       json::serializer::with_escaping_unicode(true),
                                       json::serializer::with_source_encoding<encoding::auto_utf>(),
                                       json::serializer::with_target_encoding<encoding::auto_utf>(),
                                   });
    std::cout << str << std::endl;

    v = json::parse("{\"test\": 2,\"array\":[3,2,1]}", {
                                                           json::parser::with_error_handler(nullptr),
                                                           json::parser::with_source_encoding<encoding::auto_utf>(),
                                                           json::parser::with_target_encoding<encoding::auto_utf>(),
                                                       });
    std::cout << json::dump(v) << std::endl;
    return 0;
}
