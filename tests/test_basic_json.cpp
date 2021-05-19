// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>

using namespace jsonxx;

class BasicJsonTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = { { "pi", 3.141 },
              { "happy", true },
              { "name", "Nomango" },
              { "chinese", "中文测试" },
              { "nothing", nullptr },
              { "list", { 1, 0, 2 } },
              { "object",
                {
                    { "currency", "USD" },
                    { "money", 42.99 },
                } },
              { "single_object",
                {
                    { "number", 123 },
                } } };
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

TEST(test_basic_json, test_object)
{
    ASSERT_NO_THROW(json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } }));

    // not an object
    ASSERT_DEATH(json::object({ {"1", 1}, {""} }), ".*");
    ASSERT_DEATH(json::object({ {1, ""} }), ".*");

    json j = json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
    json j2 = j;
    ASSERT_TRUE(j == j2);

    ASSERT_THROW(j[0], json_invalid_key);
    ASSERT_THROW(const_cast<const json&>(j)[0], json_invalid_key);
    ASSERT_THROW(const_cast<const json&>(j)["missing"], std::out_of_range);
}

TEST(test_basic_json, test_array)
{
    ASSERT_NO_THROW(json::array({ 1, 2, 3 }));

    json j;
    ASSERT_NO_THROW(j = json::array({ { "user", { { "id", 1 }, { "name", "Nomango" } } } }));
    ASSERT_EQ(j.is_array(), true);
    ASSERT_EQ(j.size(), 1);
    ASSERT_EQ(j[0].is_array(), true);

    ASSERT_THROW(j["test"], json_invalid_key);
    ASSERT_THROW(const_cast<const json&>(j)["test"], json_invalid_key);
    ASSERT_THROW(const_cast<const json&>(j)[1], std::out_of_range);
}

TEST(test_basic_json, test_method_size)
{
    json j;
    // string
    j = "string";
    ASSERT_EQ(j.size(), 1);
    // integer
    j = 100;
    ASSERT_EQ(j.size(), 1);
    // floating
    j = 100.0;
    ASSERT_EQ(j.size(), 1);
    // boolean
    j = true;
    ASSERT_EQ(j.size(), 1);
    // null
    j = nullptr;
    ASSERT_EQ(j.size(), 0);
    // array
    j = json::array({ 1, 2, 3 });
    ASSERT_EQ(j.size(), 3);
    // object
    j = json::object({ { "1", 1 }, { "2", 2 } });
    ASSERT_EQ(j.size(), 2);
}

TEST(test_basic_json, test_method_clear)
{
    json j;
    // string
    j = "string";
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.as_string(), "");
    // integer
    j = 100;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.as_int(), 0);
    // floating
    j = 100.0;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.as_float(), 0);
    // boolean
    j = true;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.as_bool(), false);
    // null
    j = nullptr;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.is_null(), true);
    // array
    j = json::array({ 1, 2, 3 });
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.size(), 0);
    // object
    j = json::object({ { "1", 1 }, { "2", 2 } });
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.size(), 0);
}

TEST(test_basic_json, test_int64)
{
    // issue 12
    int64_t max64 = std::numeric_limits<int64_t>::max();
    json64  j     = max64;
    ASSERT_EQ(j.as_int(), max64);
}
