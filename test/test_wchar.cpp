// Copyright (c) 2021 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>

using namespace jsonxx;

class WCharTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = {
            {L"pi", 3.141},
            {L"happy", true},
            {L"name", L"Nomango"},
            {L"nothing", nullptr},
            {L"answer", {
                {L"everything", 42}
            }},
            {L"list", {1, 0, 2}},
            {L"object", {
                {L"currency", L"USD"},
                {L"value", 42.99}
            }},
        };
    }

    wjson j;
};

TEST_F(WCharTest, test_write_to_stream)
{
    std::wstringstream ss;
    ss << j;
    ASSERT_EQ(ss.str(), j.dump());

    ss.str(L"");
    ss << std::setw(4) << j;
    ASSERT_EQ(ss.str(), j.dump(4));

    ss.str(L"");
    ss << std::setw(2) << std::setfill(L'.') << j;
    ASSERT_EQ(ss.str(), j.dump(2, '.'));
}

#include <iostream>
TEST(test_wchar, test_dump_escaped)
{
    // issue 8
    wjson j = L"我是地球🌍";
    auto s = j.dump(-1, ' ', true);
    ASSERT_EQ(j.dump(), L"\"我是地球🌍\"");
    ASSERT_EQ(j.dump(-1, ' ', false), L"\"我是地球🌍\"");
    ASSERT_EQ(j.dump(-1, ' ', true), L"\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"");
}

TEST(test_parser_w, test_parse)
{
    auto j = wjson::parse(L"{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }");
    ASSERT_EQ(j[L"happy"].as_bool(), true);
    ASSERT_DOUBLE_EQ(j[L"pi"].as_float(), 3.141);
    ASSERT_EQ(j[L"name"].as_string(), L"中文测试");
}

TEST(test_parser_w, test_parse_surrogate)
{
    // issue 8
    auto j = wjson::parse(L"\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"");
    ASSERT_EQ(j.as_string(), L"我是地球🌍");
}
