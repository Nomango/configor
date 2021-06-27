// Copyright (c) 2019 Nomango

#include "common.h"

class BasicJsonTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = json({ json("123"), 3.14 });
        j = json({ "123", 3.14 });
        j = { "123", 3.14 };
        j = { nullptr };
        j = { true };

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
    ASSERT_DOUBLE_EQ(j["pi"].get<double>(), 3.141);
    ASSERT_EQ(j["happy"].get<bool>(), true);
    ASSERT_EQ(j["name"].get<std::string>(), "Nomango");
    ASSERT_EQ(j["list"][0].get<int64_t>(), 1);
    ASSERT_EQ(j["list"][1].get<int64_t>(), 0);
    ASSERT_EQ(j["list"][2].get<int64_t>(), 2);
    ASSERT_EQ(j["single_object"]["number"].get<int64_t>(), 123);
}

TEST_F(BasicJsonTest, test_numeric_type)
{
#define TEST_NUMERIC_GET_VALUE(j, NUMERIC_TYPE, EXPECT_VALUE)                      \
    {                                                                              \
        ASSERT_EQ(j.get<NUMERIC_TYPE>(), static_cast<NUMERIC_TYPE>(EXPECT_VALUE)); \
    }

#define TEST_INTEGER_COMPATIBLE(INT_TYPE)               \
    {                                                   \
        auto i = INT_TYPE(123);                         \
        json j = i;                                     \
        ASSERT_TRUE(j.is_integer());                    \
        ASSERT_FALSE(j.is_float());                     \
        ASSERT_TRUE(j.is_number());                     \
        TEST_NUMERIC_GET_VALUE(j, int8_t, i);           \
        TEST_NUMERIC_GET_VALUE(j, int16_t, i);          \
        TEST_NUMERIC_GET_VALUE(j, int32_t, i);          \
        TEST_NUMERIC_GET_VALUE(j, int64_t, i);          \
        TEST_NUMERIC_GET_VALUE(j, uint8_t, i);          \
        TEST_NUMERIC_GET_VALUE(j, uint16_t, i);         \
        TEST_NUMERIC_GET_VALUE(j, uint32_t, i);         \
        TEST_NUMERIC_GET_VALUE(j, uint64_t, i);         \
        ASSERT_THROW(j.get<double>(), json_type_error); \
    }

#define TEST_FLOAT_COMPATIBLE(FLOAT_TYPE)                \
    {                                                    \
        auto i = FLOAT_TYPE(123.0);                      \
        json j = i;                                      \
        ASSERT_FALSE(j.is_integer());                    \
        ASSERT_TRUE(j.is_float());                       \
        ASSERT_TRUE(j.is_number());                      \
        TEST_NUMERIC_GET_VALUE(j, float, i);             \
        TEST_NUMERIC_GET_VALUE(j, double, i);            \
        ASSERT_THROW(j.get<int64_t>(), json_type_error); \
    }

    TEST_INTEGER_COMPATIBLE(int8_t);
    TEST_INTEGER_COMPATIBLE(int16_t);
    TEST_INTEGER_COMPATIBLE(int32_t);
    TEST_FLOAT_COMPATIBLE(float);
    TEST_FLOAT_COMPATIBLE(double);

    // int to float
    {
        json j = 0;
        ASSERT_DOUBLE_EQ(j.as_float(), 0);
        j = 1;
        ASSERT_DOUBLE_EQ(j.as_float(), 1.0);
    }

    // float to int
    {
        json j = 0.0;
        ASSERT_EQ(j.as_integer(), static_cast<int64_t>(0.0));
        j = 1.0;
        ASSERT_EQ(j.as_integer(), static_cast<int64_t>(1.0));
        j = 1.49;
        ASSERT_EQ(j.as_integer(), static_cast<int64_t>(1.49));
        j = 1.51;
        ASSERT_EQ(j.as_integer(), static_cast<int64_t>(1.51));
    }
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
    ASSERT_TRUE(j["pi"] == 3.141);
    ASSERT_TRUE(j["happy"] == true);
    ASSERT_TRUE(j["name"] == "Nomango");
    ASSERT_TRUE(j["list"][0].get<int64_t>() == 1);
    ASSERT_TRUE(j["list"][0].get<int64_t>() == 1u);
    ASSERT_TRUE(j["list"][0].get<int64_t>() == 1l);
    ASSERT_TRUE(j["list"][0].get<int64_t>() == int64_t(1));
}

TEST_F(BasicJsonTest, test_assign)
{
    j["happy"] = false;
    ASSERT_EQ(j["happy"].get<bool>(), false);

    j["list"][0] = -1;
    ASSERT_EQ(j["list"][0].get<int64_t>(), -1);

    j["new_item"] = "string";
    ASSERT_EQ(j["new_item"].get<std::string>(), "string");
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
    ASSERT_THROW(json::object({ { "1", 1 }, { "" } }), json_type_error);
    ASSERT_THROW(json::object({ { 1, "" } }), json_type_error);

    json j  = json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
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

TEST(test_basic_json, test_type)
{
    json j;
    // string
    j = "string";
    ASSERT_EQ(j.type(), json_type::string);
    ASSERT_STREQ(j.type_name(), "string");
    // integer
    j = 100;
    ASSERT_EQ(j.type(), json_type::number_integer);
    ASSERT_STREQ(j.type_name(), "integer");
    // floating
    j = 100.0;
    ASSERT_EQ(j.type(), json_type::number_float);
    ASSERT_STREQ(j.type_name(), "float");
    // boolean
    j = true;
    ASSERT_EQ(j.type(), json_type::boolean);
    ASSERT_STREQ(j.type_name(), "boolean");
    // null
    j = nullptr;
    ASSERT_EQ(j.type(), json_type::null);
    ASSERT_STREQ(j.type_name(), "null");
    // array
    j = json::array({ 1, 2, 3 });
    ASSERT_EQ(j.type(), json_type::array);
    ASSERT_STREQ(j.type_name(), "array");
    // object
    j = json::object({ { "1", 1 }, { "2", 2 } });
    ASSERT_EQ(j.type(), json_type::object);
    ASSERT_STREQ(j.type_name(), "object");
}

TEST(test_basic_json, test_method_clear)
{
    json j;
    // string
    j = "string";
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.get<std::string>(), "");
    // integer
    j = 100;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.get<int64_t>(), 0);
    // floating
    j = 100.0;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.get<double>(), 0);
    // boolean
    j = true;
    ASSERT_NO_THROW(j.clear());
    ASSERT_EQ(j.get<bool>(), false);
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
    json    j     = max64;
    ASSERT_EQ(j.get<int64_t>(), max64);
}

TEST(test_basic_json, test_json_value)
{
    json j;

    ASSERT_NO_THROW(j = json_type::null);
    ASSERT_EQ(j.type(), json_type::null);

    ASSERT_NO_THROW(j = json_type::boolean);
    ASSERT_EQ(j.type(), json_type::boolean);
    ASSERT_EQ(j.get<bool>(), false);

    ASSERT_NO_THROW(j = json_type::number_integer);
    ASSERT_EQ(j.type(), json_type::number_integer);
    ASSERT_EQ(j.get<json::integer_type>(), 0);

    ASSERT_NO_THROW(j = json_type::number_float);
    ASSERT_EQ(j.type(), json_type::number_float);
    ASSERT_EQ(j.get<json::float_type>(), 0.0);

    ASSERT_NO_THROW(j = json_type::string);
    ASSERT_EQ(j.type(), json_type::string);
    ASSERT_EQ(j.get<json::string_type>(), std::string{});

    ASSERT_NO_THROW(j = json_type::object);
    ASSERT_EQ(j.type(), json_type::object);
    ASSERT_EQ(j.get<json::object_type>(), json::object_type{});

    ASSERT_NO_THROW(j = json_type::array);
    ASSERT_EQ(j.type(), json_type::array);
    ASSERT_EQ(j.get<json::array_type>(), json::array_type{});
}
