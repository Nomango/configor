// Copyright (c) 2019 Nomango

#include "common.h"

class BasicJsonTest
{
protected:
    BasicJsonTest()
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

TEST_CASE_METHOD(BasicJsonTest, "test_type")
{
    const auto& j = this->j;
    CHECK(j.is_object());
    CHECK(j["pi"].is_float());
    CHECK(j["happy"].is_bool());
    CHECK(j["name"].is_string());
    CHECK(j["nothing"].is_null());
    CHECK(j["list"].is_array());
    CHECK(j["object"].is_object());
    CHECK(j["object"]["currency"].is_string());
    CHECK(j["object"]["money"].is_float());
    CHECK(j["single_object"]["number"].is_number());
    CHECK_THROWS_AS(j["missing"].is_null(), std::out_of_range);
    CHECK(this->j["missing"].is_null());
}

TEST_CASE_METHOD(BasicJsonTest, "test_get")
{
    CHECK(j["pi"].get<double>() == Approx(3.141));
    CHECK(j["happy"].get<bool>());
    CHECK(j["name"].get<std::string>() == "Nomango");
    CHECK(j["list"][0].get<int64_t>() == 1);
    CHECK(j["list"][1].get<int64_t>() == 0);
    CHECK(j["list"][2].get<int64_t>() == 2);
    CHECK(j["single_object"]["number"].get<int64_t>() == 123);
}

TEST_CASE_METHOD(BasicJsonTest, "test_try_get")
{
    double d = 0;
    CHECK(j["pi"].try_get(d));
    CHECK(d == Approx(3.141));

    int i = 1;
    CHECK_FALSE(j["pi"].try_get(i));
    CHECK(i == 1);
}

TEST_CASE_METHOD(BasicJsonTest, "test_numeric_type")
{
#define TEST_NUMERIC_GET_VALUE(j, NUMERIC_TYPE, EXPECT_VALUE)                    \
    {                                                                            \
        CHECK(j.get<NUMERIC_TYPE>() == static_cast<NUMERIC_TYPE>(EXPECT_VALUE)); \
    }

#define TEST_INTEGER_COMPATIBLE(INT_TYPE)                      \
    {                                                          \
        auto i = INT_TYPE(123);                                \
        json j = i;                                            \
        CHECK(j.is_integer());                                 \
        CHECK_FALSE(j.is_float());                             \
        CHECK(j.is_number());                                  \
        TEST_NUMERIC_GET_VALUE(j, int8_t, i);                  \
        TEST_NUMERIC_GET_VALUE(j, int16_t, i);                 \
        TEST_NUMERIC_GET_VALUE(j, int32_t, i);                 \
        TEST_NUMERIC_GET_VALUE(j, int64_t, i);                 \
        TEST_NUMERIC_GET_VALUE(j, uint8_t, i);                 \
        TEST_NUMERIC_GET_VALUE(j, uint16_t, i);                \
        TEST_NUMERIC_GET_VALUE(j, uint32_t, i);                \
        TEST_NUMERIC_GET_VALUE(j, uint64_t, i);                \
        CHECK_THROWS_AS(j.get<double>(), configor_type_error); \
    }

#define TEST_FLOAT_COMPATIBLE(FLOAT_TYPE)                       \
    {                                                           \
        auto i = FLOAT_TYPE(123.0);                             \
        json j = i;                                             \
        CHECK_FALSE(j.is_integer());                            \
        CHECK(j.is_float());                                    \
        CHECK(j.is_number());                                   \
        TEST_NUMERIC_GET_VALUE(j, float, i);                    \
        TEST_NUMERIC_GET_VALUE(j, double, i);                   \
        CHECK_THROWS_AS(j.get<int64_t>(), configor_type_error); \
    }

    TEST_INTEGER_COMPATIBLE(int8_t);
    TEST_INTEGER_COMPATIBLE(int16_t);
    TEST_INTEGER_COMPATIBLE(int32_t);
    TEST_FLOAT_COMPATIBLE(float);
    TEST_FLOAT_COMPATIBLE(double);

    // int to float
    {
        json j = 0;
        CHECK(j.as_float() == Approx(0));
        j = 1;
        CHECK(j.as_float() == Approx(1.0));
    }

    // float to int
    {
        json j = 0.0;
        CHECK(j.as_integer() == static_cast<int64_t>(0.0));
        j = 1.0;
        CHECK(j.as_integer() == static_cast<int64_t>(1.0));
        j = 1.49;
        CHECK(j.as_integer() == static_cast<int64_t>(1.49));
        j = 1.51;
        CHECK(j.as_integer() == static_cast<int64_t>(1.51));
    }
}

TEST_CASE_METHOD(BasicJsonTest, "test_equal")
{
    CHECK(j["pi"] == 3.141);
    CHECK(j["pi"] > 3);
    CHECK(j["pi"] < 4);
    CHECK(j["happy"] == true);
    CHECK(j["name"] == "Nomango");
    CHECK(j["list"][0] == 1);
    CHECK(j["list"][0] == 1u);
    CHECK(j["list"][0] == 1l);
    CHECK(j["list"][0] == int64_t(1));
    CHECK(j["list"][1] == 0);
    CHECK(j["list"][2] == 2);
    CHECK(j["list"][0] > 0);
    CHECK(j["list"][0] < 2);
    CHECK(j["single_object"]["number"] == 123);

    CHECK(json(1.0) == 1);
    CHECK(json(1) == 1.0);
    CHECK(json(1.0) == 1.0f);
    CHECK(json(1.0f) == 1.0);
}

TEST_CASE_METHOD(BasicJsonTest, "test_explicit_convert")
{
    CHECK(j["pi"] == 3.141);
    CHECK(j["happy"] == true);
    CHECK(j["name"] == "Nomango");
    CHECK(j["list"][0].get<int64_t>() == 1);
    CHECK(j["list"][0].get<int64_t>() == 1u);
    CHECK(j["list"][0].get<int64_t>() == 1l);
    CHECK(j["list"][0].get<int64_t>() == int64_t(1));
}

TEST_CASE_METHOD(BasicJsonTest, "test_assign")
{
    j["happy"] = false;
    CHECK_FALSE(j["happy"].get<bool>());

    j["list"][0] = -1;
    CHECK(j["list"][0].get<int64_t>() == -1);

    j["new_item"] = "string";
    CHECK(j["new_item"].get<std::string>() == "string");
}

TEST_CASE("test_basic_json")
{
    SECTION("test_object")
    {
        CHECK_NOTHROW(json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } }));

        // not an object
        CHECK_THROWS_AS(json::object({ { "1", 1 }, { "" } }), configor_type_error);
        CHECK_THROWS_AS(json::object({ { 1, "" } }), configor_type_error);

        json j  = json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
        json j2 = j;
        CHECK(j == j2);

        CHECK_THROWS_AS(j[0], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const json&>(j)[0], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const json&>(j)["missing"], std::out_of_range);
    }

    SECTION("test_array")
    {
        CHECK_NOTHROW(json::array({ 1, 2, 3 }));

        json j;
        CHECK_NOTHROW(j = json::array({ { "user", { { "id", 1 }, { "name", "Nomango" } } } }));
        CHECK(j.is_array());
        CHECK(j.size() == 1);
        CHECK(j[0].is_array());

        CHECK_THROWS_AS(j["test"], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const json&>(j)["test"], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const json&>(j)[1], std::out_of_range);
    }

    SECTION("test_method_size")
    {
        json j;
        // string
        j = "string";
        CHECK(j.size() == 1);
        // integer
        j = 100;
        CHECK(j.size() == 1);
        // floating
        j = 100.0;
        CHECK(j.size() == 1);
        // boolean
        j = true;
        CHECK(j.size() == 1);
        // null
        j = nullptr;
        CHECK(j.size() == 0);
        // array
        j = json::array({ 1, 2, 3 });
        CHECK(j.size() == 3);
        // object
        j = json::object({ { "1", 1 }, { "2", 2 } });
        CHECK(j.size() == 2);
    }

    SECTION("test_type")
    {
        using Catch::Matchers::Equals;

        json j;
        // string
        j = "string";
        CHECK(j.type() == config_value_type::string);
        CHECK_THAT(j.type_name(), Equals("string"));
        // integer
        j = 100;
        CHECK(j.type() == config_value_type::number_integer);
        CHECK_THAT(j.type_name(), Equals("integer"));
        // floating
        j = 100.0;
        CHECK(j.type() == config_value_type::number_float);
        CHECK_THAT(j.type_name(), Equals("float"));
        // boolean
        j = true;
        CHECK(j.type() == config_value_type::boolean);
        CHECK_THAT(j.type_name(), Equals("boolean"));
        // null
        j = nullptr;
        CHECK(j.type() == config_value_type::null);
        CHECK_THAT(j.type_name(), Equals("null"));
        // array
        j = json::array({ 1, 2, 3 });
        CHECK(j.type() == config_value_type::array);
        CHECK_THAT(j.type_name(), Equals("array"));
        // object
        j = json::object({ { "1", 1 }, { "2", 2 } });
        CHECK(j.type() == config_value_type::object);
        CHECK_THAT(j.type_name(), Equals("object"));
    }

    SECTION("test_method_clear")
    {
        json j;
        // string
        j = "string";
        CHECK_NOTHROW(j.clear());
        CHECK(j.get<std::string>() == "");
        // integer
        j = 100;
        CHECK_NOTHROW(j.clear());
        CHECK(j.get<int64_t>() == 0);
        // floating
        j = 100.0;
        CHECK_NOTHROW(j.clear());
        CHECK(j.get<double>() == 0);
        // boolean
        j = true;
        CHECK_NOTHROW(j.clear());
        CHECK_FALSE(j.get<bool>());
        // null
        j = nullptr;
        CHECK_NOTHROW(j.clear());
        CHECK(j.is_null());
        // array
        j = json::array({ 1, 2, 3 });
        CHECK_NOTHROW(j.clear());
        CHECK(j.size() == 0);
        // object
        j = json::object({ { "1", 1 }, { "2", 2 } });
        CHECK_NOTHROW(j.clear());
        CHECK(j.size() == 0);
    }

    SECTION("test_int64")
    {
        // issue 12
        int64_t max64 = std::numeric_limits<int64_t>::max();
        json    j     = max64;
        CHECK(j.get<int64_t>() == max64);
    }

    SECTION("test_json_value")
    {
        json j;

        CHECK_NOTHROW(j = config_value_type::null);
        CHECK(j.type() == config_value_type::null);

        CHECK_NOTHROW(j = config_value_type::boolean);
        CHECK(j.type() == config_value_type::boolean);
        CHECK_FALSE(j.get<bool>());

        CHECK_NOTHROW(j = config_value_type::number_integer);
        CHECK(j.type() == config_value_type::number_integer);
        CHECK(j.get<json::integer_type>() == 0);

        CHECK_NOTHROW(j = config_value_type::number_float);
        CHECK(j.type() == config_value_type::number_float);
        CHECK(j.get<json::float_type>() == 0.0);

        CHECK_NOTHROW(j = config_value_type::string);
        CHECK(j.type() == config_value_type::string);
        CHECK(j.get<json::string_type>() == std::string{});

        CHECK_NOTHROW(j = config_value_type::array);
        CHECK(j.type() == config_value_type::array);
        CHECK(j.get<json::array_type>() == json::array_type{});

        CHECK_NOTHROW(j = config_value_type::object);
        CHECK(j.type() == config_value_type::object);
        CHECK(j.get<json::object_type>() == json::object_type{});
    }
}
