// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <array>
#include <fstream>

using namespace jsonxx;

TEST(test_parser, test_parse)
{
    auto j = json::parse("{ \"happy\": true, \"pi\": 3.141 }");
    (void)j;
}

TEST(test_parser, test_read_from_file)
{
    std::array<std::string, 5> files = {
        "data/json.org/1.json",
        "data/json.org/2.json",
        "data/json.org/3.json",
        "data/json.org/4.json",
        "data/json.org/5.json",
    };

    for (const auto& file : files)
    {
        // read a json file
        std::ifstream ifs(file);

        json j;
        ifs >> j;
    }
}
