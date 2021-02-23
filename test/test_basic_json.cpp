// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <iostream>

using namespace jsonxx;

TEST(test_basic_json, test_assign)
{
    json j;

    // add a number that is stored as double (note the implicit conversion of j to an object)
    j["pi"] = 3.141;

    // add a Boolean that is stored as bool
    j["happy"] = true;

    // add a string that is stored as std::string
    j["name"] = "Nomango";

    // add another null object by passing nullptr
    j["nothing"] = nullptr;

    // add an object inside the object
    j["answer"]["everything"] = 42;

    // add an array that is stored as std::vector (using an initializer list)
    j["list"] = { 1, 0, 2 };

    // add another object (using an initializer list of pairs)
    j["object"] = { {"currency", "USD"}, {"value", 42.99} };
}

class BasicJsonTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Nomango"},
            {"nothing", nullptr},
            {"answer", {
                    {"everything", 42}
            }},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"value", 42.99}
            }}
        };
    }

    json j;
};

TEST_F(BasicJsonTest, test_type)
{
    ASSERT_EQ(j.is_object(), true);
    ASSERT_EQ(j["pi"].is_float(), true);
    ASSERT_EQ(j["happy"].is_bool(), true);
    ASSERT_EQ(j["name"].is_string(), true);
    ASSERT_EQ(j["nothing"].is_null(), true);
    ASSERT_EQ(j["answer"].is_object(), true);
    ASSERT_EQ(j["answer"]["everything"].is_number(), true);
    ASSERT_EQ(j["list"].is_array(), true);
}

TEST_F(BasicJsonTest, test_get)
{
    ASSERT_DOUBLE_EQ(j["pi"].as_float(), 3.141);
    ASSERT_EQ(j["happy"].as_bool(), true);
    ASSERT_EQ(j["name"].as_string(), "Nomango");
    ASSERT_EQ(j["answer"]["everything"].as_int(), 42);
    ASSERT_EQ(j["list"][0].as_int(), 1);
    ASSERT_EQ(j["list"][1].as_int(), 0);
    ASSERT_EQ(j["list"][2].as_int(), 2);
}

TEST_F(BasicJsonTest, test_dump)
{
    ASSERT_EQ(j.dump(), "{\"answer\":{\"everything\":42},\"happy\":true,\"list\":[1,0,2],\"name\":\"Nomango\",\"nothing\":null,\"object\":{\"currency\":\"USD\",\"value\":42.990000000000002},\"pi\":3.141}");
    // for-each
    for (const auto& v : j)
    {
        (void)v.dump();
    }
}

TEST_F(BasicJsonTest, test_iterator_dump)
{
    // iterator
    for (auto iter = j.begin(); iter != j.end(); iter++)
    {
        (void)iter->dump();
    }
}
