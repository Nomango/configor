// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <array>
#include <fstream>
#include <functional>

using namespace jsonxx;

TEST(test_parser, test_parse)
{
    auto j = json::parse("{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }");
    ASSERT_EQ(j["happy"].as_bool(), true);
    ASSERT_DOUBLE_EQ(j["pi"].as_float(), 3.141);
    ASSERT_EQ(j["name"].as_string(), "中文测试");

    // issue 4
    ASSERT_NO_THROW(json::parse("{\"empty\": []}"));
    ASSERT_NO_THROW(json::parse("{}"));
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

    std::function<void(json&)> tests[] = {
        [](json& j) {
            // test 1
            auto list = j["glossary"]["GlossDiv"]["GlossList"]["GlossEntry"]["GlossDef"]["GlossSeeAlso"];
            ASSERT_EQ(list[0].as_string(), "GML");
            ASSERT_EQ(list[1].as_string(), "XML");
        },
        [](json& j) {
            // test 2
            ASSERT_EQ(j["menu"]["popup"]["menuitem"][0]["onclick"].as_string(), "CreateNewDoc()");
        },
        [](json& j) {
            // test 3
        },
        [](json& j) {
            // test 4
        },
        [](json& j) {
            // test 5
            ASSERT_EQ(j["menu"]["items"][2].is_null(), true);
            ASSERT_EQ(j["menu"]["items"][3]["id"].as_string(), "ZoomIn");
        },
    };

    for (size_t i = 0; i < files.size(); i++)
    {
        // read a json file
        std::ifstream ifs(files[i]);

        json j;
        ifs >> j;

        // run tests
        tests[i](j);
    }
}
