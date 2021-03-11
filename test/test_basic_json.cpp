// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <iostream>

using namespace jsonxx;

class BasicJsonTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Nomango"},
            {"chinese", "中文测试"},
            {"nothing", nullptr},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"money", 42.99},
            }},
            {"single_object", {
                {"number", 123},
            }}
        };
    }

    json j;
};

TEST_F(BasicJsonTest, test_type)
{
    const auto& j = this->j;
    ASSERT_EQ(j.is_object(), true);
    ASSERT_EQ(j["pi"].is_float(), true);
    ASSERT_EQ(j["happy"].is_bool(), true);
    ASSERT_EQ(j["name"].is_string(), true);
    ASSERT_EQ(j["nothing"].is_null(), true);
    ASSERT_EQ(j["list"].is_array(), true);
    ASSERT_EQ(j["object"].is_object(), true);
    ASSERT_EQ(j["object"]["currency"].is_string(), true);
    ASSERT_EQ(j["object"]["money"].is_float(), true);
    ASSERT_EQ(j["single_object"]["number"].is_number(), true);
    ASSERT_THROW(j["missing"].is_null(), std::out_of_range);
    ASSERT_EQ(this->j["missing"].is_null(), true);
}

TEST_F(BasicJsonTest, test_get)
{
    ASSERT_DOUBLE_EQ(j["pi"].as_float(), 3.141);
    ASSERT_EQ(j["happy"].as_bool(), true);
    ASSERT_EQ(j["name"].as_string(), "Nomango");
    ASSERT_EQ(j["list"][0].as_int(), 1);
    ASSERT_EQ(j["list"][1].as_int(), 0);
    ASSERT_EQ(j["list"][2].as_int(), 2);
    ASSERT_EQ(j["single_object"]["number"].as_int(), 123);
}

TEST_F(BasicJsonTest, test_equal)
{
    ASSERT_TRUE(j["pi"] == 3.141);
    ASSERT_TRUE(j["pi"] > 3);
    ASSERT_TRUE(j["pi"] < 4);
    ASSERT_TRUE(j["happy"] == true);
    ASSERT_TRUE(j["name"] == "Nomango");
    ASSERT_TRUE(j["list"][0] == 1);
    ASSERT_TRUE(j["list"][0] == 1u);
    ASSERT_TRUE(j["list"][0] == 1l);
    ASSERT_TRUE(j["list"][0] == int64_t(1));
    ASSERT_TRUE(j["list"][1] == 0);
    ASSERT_TRUE(j["list"][2] == 2);
    ASSERT_TRUE(j["list"][0] > 0);
    ASSERT_TRUE(j["list"][0] < 2);
    ASSERT_TRUE(j["single_object"]["number"] == 123);

    ASSERT_TRUE(json(1.0) == 1);
    ASSERT_TRUE(json(1) == 1.0);
    ASSERT_TRUE(json(1.0) == 1.0f);
    ASSERT_TRUE(json(1.0f) == 1.0);
}

TEST_F(BasicJsonTest, test_explicit_convert)
{
    ASSERT_TRUE((double)j["pi"] == 3.141);
    ASSERT_TRUE((float)j["pi"] == 3.141f);
    ASSERT_TRUE((bool)j["happy"] == true);
    ASSERT_TRUE((std::string)j["name"] == "Nomango");
    ASSERT_TRUE((int)j["list"][0].as_int() == 1);
    ASSERT_TRUE((unsigned int)j["list"][0].as_int() == 1u);
    ASSERT_TRUE((long)j["list"][0].as_int() == 1l);
    ASSERT_TRUE((int64_t)j["list"][0].as_int() == int64_t(1));
}

TEST_F(BasicJsonTest, test_assign)
{
    j["happy"] = false;
    ASSERT_EQ(j["happy"].as_bool(), false);

    j["list"][0] = -1;
    ASSERT_EQ(j["list"][0].as_int(), -1);

    j["new_item"] = "string";
    ASSERT_EQ(j["new_item"].as_string(), "string");
}

TEST_F(BasicJsonTest, test_dump)
{
    ASSERT_EQ(j["pi"].dump(), "3.141");
    ASSERT_EQ(j["happy"].dump(), "true");
    ASSERT_EQ(j["name"].dump(), "\"Nomango\"");
    ASSERT_EQ(j["chinese"].dump(), "\"中文测试\"");
    ASSERT_EQ(j["nothing"].dump(), "null");
    ASSERT_EQ(j["list"].dump(), "[1,0,2]");
    ASSERT_EQ(j["single_object"].dump(), "{\"number\":123}");
}

TEST_F(BasicJsonTest, test_iterator)
{
    for (auto iter = j.begin(); iter != j.end(); iter++)
    {
        ASSERT_EQ(iter.value(), j[iter.key()]);
    }
    ASSERT_TRUE(j.find("pi") != j.cend());
    ASSERT_TRUE(j.find("missing") == j.cend());
}

TEST_F(BasicJsonTest, test_object)
{
    ASSERT_EQ(j.size(), 8);
    ASSERT_EQ(j["object"].size(), 2);
    ASSERT_EQ(j["single_object"].size(), 1);
    ASSERT_EQ(j["list"].size(), 3);
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.size(), 0);
}
