// Copyright (c) 2019 Nomango

#include <array>
#include <fstream>
#include <functional>
#include <gtest/gtest.h>
#include <jsonxx/json.hpp>

using namespace jsonxx;

TEST(test_parser, test_parse)
{
    auto j = json::parse("{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }");
    ASSERT_EQ(j["happy"].as_bool(), true);
    ASSERT_DOUBLE_EQ(j["pi"].as_float(), 3.141);
    ASSERT_EQ(j["name"].as_string(), "中文测试");

    // parse empty object
    // issue 4
    ASSERT_NO_THROW(j = json::parse("{}"));
    ASSERT_TRUE(j.is_object() && j.empty());

    // parse empty array
    ASSERT_NO_THROW(j = json::parse("[]"));
    ASSERT_TRUE(j.is_array() && j.empty());

    // parse integer
    ASSERT_EQ(json::parse("0").as_integer(), 0);
    ASSERT_EQ(json::parse("2147483647").as_integer(), int32_t(2147483647));
    ASSERT_EQ(json64::parse("9223372036854775807").as_integer(), int64_t(9223372036854775807));

    // parse signed integer
    ASSERT_EQ(json::parse("+0").as_integer(), 0);
    ASSERT_EQ(json::parse("+2147483647").as_integer(), int32_t(2147483647));
    ASSERT_EQ(json64::parse("+9223372036854775807").as_integer(), int64_t(9223372036854775807));
    ASSERT_EQ(json::parse("-0").as_integer(), 0);
    ASSERT_EQ(json::parse("-2147483647").as_integer(), int32_t(-2147483647));
    ASSERT_EQ(json64::parse("-9223372036854775807").as_integer(), int64_t(-9223372036854775807));

    // parse float
    ASSERT_DOUBLE_EQ(json::parse("0.25").as_float(), 0.25);
    ASSERT_DOUBLE_EQ(json::parse("1.25").as_float(), 1.25);
    ASSERT_DOUBLE_EQ(json::parse("1.125e2").as_float(), 112.5);
    ASSERT_DOUBLE_EQ(json::parse("0.125e2").as_float(), 12.5);
    ASSERT_DOUBLE_EQ(json::parse("112.5e-2").as_float(), 1.125);
    ASSERT_DOUBLE_EQ(json::parse("12.5e-2").as_float(), 0.125);

    // parse signed float
    ASSERT_DOUBLE_EQ(json::parse("+0.25").as_float(), 0.25);
    ASSERT_DOUBLE_EQ(json::parse("+1.25").as_float(), 1.25);
    ASSERT_DOUBLE_EQ(json::parse("+1.125e2").as_float(), 112.5);
    ASSERT_DOUBLE_EQ(json::parse("+0.125e2").as_float(), 12.5);
    ASSERT_DOUBLE_EQ(json::parse("+112.5e-2").as_float(), 1.125);
    ASSERT_DOUBLE_EQ(json::parse("+12.5e-2").as_float(), 0.125);

    ASSERT_DOUBLE_EQ(json::parse("-0.25").as_float(), -0.25);
    ASSERT_DOUBLE_EQ(json::parse("-1.25").as_float(), -1.25);
    ASSERT_DOUBLE_EQ(json::parse("-1.125e2").as_float(), -112.5);
    ASSERT_DOUBLE_EQ(json::parse("-0.125e2").as_float(), -12.5);
    ASSERT_DOUBLE_EQ(json::parse("-112.5e-2").as_float(), -1.125);
    ASSERT_DOUBLE_EQ(json::parse("-12.5e-2").as_float(), -0.125);

    // parse controle characters
    ASSERT_THROW(json::parse("\t"), json_parse_error);
    ASSERT_THROW(json::parse("\r"), json_parse_error);
    ASSERT_THROW(json::parse("\n"), json_parse_error);
    ASSERT_THROW(json::parse("\b"), json_parse_error);
    ASSERT_THROW(json::parse("\f"), json_parse_error);

    // unexpect end
    ASSERT_THROW(json::parse("\\"), json_parse_error);
}

TEST(test_parser, test_comment)
{
    auto j = json::parse(R"(// some comments
    /* some comments */
    {
        // some comments
        /* some comments */ "happy": true,  /* some comments */
        // "pi": 1,
        "pi": 3.141, // some comments
        // "pi": 2,
        /*
        some comments
        "pi": 3,
        */"name": "中文测试"
    }// some comments)");
    ASSERT_EQ(j["happy"].as_bool(), true);
    ASSERT_DOUBLE_EQ(j["pi"].as_float(), 3.141);
    ASSERT_EQ(j["name"].as_string(), "中文测试");
}

TEST(test_parser, test_parse_surrogate)
{
    // issue 8
    auto j = json::parse("\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"");
    ASSERT_EQ(j.as_string(), "我是地球🌍");
}

TEST(test_parser, test_read_from_file)
{
    std::array<std::string, 5> files = {
        "tests/data/json.org/1.json", "tests/data/json.org/2.json", "tests/data/json.org/3.json",
        "tests/data/json.org/4.json", "tests/data/json.org/5.json",
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
