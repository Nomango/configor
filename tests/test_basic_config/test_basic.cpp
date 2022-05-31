// Copyright (c) 2019 Nomango

#include "common.h"

class BasicConfigTest
{
protected:
    BasicConfigTest()
    {
        c = { { "pi", 3.141 },
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

    config c;
};

TEST_CASE_METHOD(BasicConfigTest, "test_type")
{
    const auto& c = this->c;
    CHECK(c.is_object());
    CHECK(c["pi"].is_float());
    CHECK(c["happy"].is_bool());
    CHECK(c["name"].is_string());
    CHECK(c["nothing"].is_null());
    CHECK(c["list"].is_array());
    CHECK(c["object"].is_object());
    CHECK(c["object"]["currency"].is_string());
    CHECK(c["object"]["money"].is_float());
    CHECK(c["single_object"]["number"].is_number());
    CHECK_THROWS_AS(c["missing"].is_null(), std::out_of_range);
    CHECK(this->c["missing"].is_null());
}

TEST_CASE_METHOD(BasicConfigTest, "test_get")
{
    CHECK(c["pi"].get<double>() == Approx(3.141));
    CHECK(c["happy"].get<bool>());
    CHECK(c["name"].get<std::string>() == "Nomango");
    CHECK(c["list"][0].get<int64_t>() == 1);
    CHECK(c["list"][1].get<int64_t>() == 0);
    CHECK(c["list"][2].get<int64_t>() == 2);
    CHECK(c["single_object"]["number"].get<int64_t>() == 123);
}

TEST_CASE_METHOD(BasicConfigTest, "test_get_ptr")
{
    CHECK(*c["happy"].get<const bool*>());
    CHECK_NOTHROW(*c["happy"].get<bool*>() = false);
    CHECK(!c["happy"].get<bool>());

    CHECK(*c["single_object"]["number"].get<const int64_t*>() == 123);
    CHECK_NOTHROW(*c["single_object"]["number"].get<int64_t*>() = 1);
    CHECK(c["single_object"]["number"].get<int64_t>() == 1);

    CHECK(*c["pi"].get<const double*>() == Approx(3.141));
    CHECK_NOTHROW(*c["pi"].get<double*>() = 1.0);
    CHECK(c["pi"].get<double>() == Approx(1.0));

    CHECK(*c["name"].get<const std::string*>() == "Nomango");
    CHECK_NOTHROW(*c["name"].get<std::string*>() = "test");
    CHECK(c["name"].get<std::string>() == "test");

    CHECK(*c["list"].get<const config::array_type*>() == c["list"]);
    CHECK_NOTHROW(*c["list"].get<config::array_type*>() = config::array_type{ 5 });
    CHECK(c["list"].get<config::array_type>() == config::array_type{ 5 });

    CHECK(*c["single_object"].get<const config::object_type*>() == c["single_object"]);
    CHECK_NOTHROW(*c["single_object"].get<config::object_type*>() = config::object_type{});
    CHECK(c["single_object"].get<config::object_type>() == config::object_type{});
}

TEST_CASE_METHOD(BasicConfigTest, "test_get_reference")
{
    CHECK(c["happy"].get<const bool&>());
    CHECK_NOTHROW(c["happy"].get<bool&>() = false);
    CHECK(!c["happy"].get<bool>());

    CHECK(c["single_object"]["number"].get<const int64_t&>() == 123);
    CHECK_NOTHROW(c["single_object"]["number"].get<int64_t&>() = 1);
    CHECK(c["single_object"]["number"].get<int64_t>() == 1);

    CHECK(c["pi"].get<const double&>() == Approx(3.141));
    CHECK_NOTHROW(c["pi"].get<double&>() = 1.0);
    CHECK(c["pi"].get<double>() == Approx(1.0));

    CHECK(c["name"].get<const std::string&>() == "Nomango");
    CHECK_NOTHROW(c["name"].get<std::string&>() = "test");
    CHECK(c["name"].get<std::string>() == "test");

    CHECK(c["list"].get<const config::array_type&>() == c["list"]);
    CHECK_NOTHROW(c["list"].get<config::array_type&>() = config::array_type{ 5 });
    CHECK(c["list"].get<config::array_type>() == config::array_type{ 5 });

    CHECK(c["single_object"].get<const config::object_type&>() == c["single_object"]);
    CHECK_NOTHROW(c["single_object"].get<config::object_type&>() = config::object_type{});
    CHECK(c["single_object"].get<config::object_type>() == config::object_type{});
}

TEST_CASE_METHOD(BasicConfigTest, "test_try_get")
{
    double d = 0;
    CHECK(c["pi"].get(d));
    CHECK(d == Approx(3.141));

    int i = 1;
    CHECK_FALSE(c["pi"].get(i));
    CHECK(i == 1);
}

TEST_CASE_METHOD(BasicConfigTest, "test_numeric_type")
{
#define TEST_NUMERIC_GET_VALUE(c, NUMERIC_TYPE, EXPECT_VALUE)                    \
    {                                                                            \
        CHECK(c.get<NUMERIC_TYPE>() == static_cast<NUMERIC_TYPE>(EXPECT_VALUE)); \
    }

#define TEST_INTEGER_COMPATIBLE(INT_TYPE)                      \
    {                                                          \
        auto   i = INT_TYPE(123);                              \
        config c = i;                                          \
        CHECK(c.is_integer());                                 \
        CHECK_FALSE(c.is_float());                             \
        CHECK(c.is_number());                                  \
        TEST_NUMERIC_GET_VALUE(c, int8_t, i);                  \
        TEST_NUMERIC_GET_VALUE(c, int16_t, i);                 \
        TEST_NUMERIC_GET_VALUE(c, int32_t, i);                 \
        TEST_NUMERIC_GET_VALUE(c, int64_t, i);                 \
        TEST_NUMERIC_GET_VALUE(c, uint8_t, i);                 \
        TEST_NUMERIC_GET_VALUE(c, uint16_t, i);                \
        TEST_NUMERIC_GET_VALUE(c, uint32_t, i);                \
        TEST_NUMERIC_GET_VALUE(c, uint64_t, i);                \
        CHECK_THROWS_AS(c.get<double>(), configor_type_error); \
    }

#define TEST_FLOAT_COMPATIBLE(FLOAT_TYPE)                       \
    {                                                           \
        auto   i = FLOAT_TYPE(123.0);                           \
        config c = i;                                           \
        CHECK_FALSE(c.is_integer());                            \
        CHECK(c.is_float());                                    \
        CHECK(c.is_number());                                   \
        TEST_NUMERIC_GET_VALUE(c, float, i);                    \
        TEST_NUMERIC_GET_VALUE(c, double, i);                   \
        CHECK_THROWS_AS(c.get<int64_t>(), configor_type_error); \
    }

    TEST_INTEGER_COMPATIBLE(int8_t);
    TEST_INTEGER_COMPATIBLE(int16_t);
    TEST_INTEGER_COMPATIBLE(int32_t);
    TEST_FLOAT_COMPATIBLE(float);
    TEST_FLOAT_COMPATIBLE(double);

    // int to float
    {
        config c = 0;
        CHECK(c.as_float() == Approx(0));
        c = 1;
        CHECK(c.as_float() == Approx(1.0));
    }

    // float to int
    {
        config c = 0.0;
        CHECK(c.as_integer() == static_cast<int64_t>(0.0));
        c = 1.0;
        CHECK(c.as_integer() == static_cast<int64_t>(1.0));
        c = 1.49;
        CHECK(c.as_integer() == static_cast<int64_t>(1.49));
        c = 1.51;
        CHECK(c.as_integer() == static_cast<int64_t>(1.51));
    }
}

TEST_CASE_METHOD(BasicConfigTest, "test_compare")
{
    CHECK(c["pi"] == 3.141);
    CHECK(c["pi"] > 3);
    CHECK(c["pi"] < 4);
    CHECK(c["happy"] == true);
    CHECK(c["name"] == "Nomango");
    CHECK(c["list"][0] == 1);
    CHECK(c["list"][0] == 1u);
    CHECK(c["list"][0] == 1l);
    CHECK(c["list"][0] == int64_t(1));
    CHECK(c["list"][1] == 0);
    CHECK(c["list"][2] == 2);
    CHECK(c["list"][0] > 0);
    CHECK(c["list"][0] < 2);
    CHECK(c["single_object"]["number"] == 123);

    CHECK(config(1.0) == 1);
    CHECK(config(1) == 1.0);
    CHECK(config(1.0) == 1.0f);
    CHECK(config(1.0f) == 1.0);
}

TEST_CASE_METHOD(BasicConfigTest, "test_explicit_convert")
{
    CHECK(c["pi"] == 3.141);
    CHECK(c["happy"] == true);
    CHECK(c["name"] == "Nomango");
    CHECK(c["list"][0].get<int64_t>() == 1);
    CHECK(c["list"][0].get<int64_t>() == 1u);
    CHECK(c["list"][0].get<int64_t>() == 1l);
    CHECK(c["list"][0].get<int64_t>() == int64_t(1));
}

TEST_CASE_METHOD(BasicConfigTest, "test_assign")
{
    c["happy"] = false;
    CHECK_FALSE(c["happy"].get<bool>());

    c["list"][0] = -1;
    CHECK(c["list"][0].get<int64_t>() == -1);

    c["new_item"] = "string";
    CHECK(c["new_item"].get<std::string>() == "string");
}

TEST_CASE("test_basic_config")
{
    SECTION("test_object")
    {
        CHECK_NOTHROW(config::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } }));

        // not an object
        CHECK_THROWS_AS(config::object({ { "1", 1 }, { "" } }), configor_type_error);
        CHECK_THROWS_AS(config::object({ { 1, "" } }), configor_type_error);

        config c  = config::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
        config j2 = c;
        CHECK(c == j2);

        CHECK_THROWS_AS(c[0], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const config&>(c)[0], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const config&>(c)["missing"], std::out_of_range);
    }

    SECTION("test_array")
    {
        CHECK_NOTHROW(config::array({ 1, 2, 3 }));

        config c;
        CHECK_NOTHROW(c = config::array({ { "user", { { "id", 1 }, { "name", "Nomango" } } } }));
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0].is_array());

        CHECK_THROWS_AS(c["test"], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const config&>(c)["test"], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const config&>(c)[1], std::out_of_range);
    }

    SECTION("test_as_bool")
    {
        // string
        CHECK(!config("").as_bool());
        CHECK(config("string").as_bool());
        // integer
        CHECK(!config(0).as_bool());
        CHECK(config(1).as_bool());
        // floating
        CHECK(!config(0.0).as_bool());
        CHECK(config(1.0).as_bool());
        // boolean
        CHECK(!config(false).as_bool());
        CHECK(config(true).as_bool());
        // null
        CHECK(!config(nullptr).as_bool());
        // array
        CHECK(!config::array({}).as_bool());
        CHECK(config::array({1}).as_bool());
        // object
        CHECK(!config::object({}).as_bool());
        CHECK(config::object({{"1", 1}}).as_bool());
    }

    SECTION("test_as_integer")
    {
        // string
        CHECK_THROWS_AS(config("12").as_integer(), configor_type_error);
        // integer
        CHECK(config(123).as_integer() == 123);
        // floating
        CHECK(config(123.334).as_integer() == config::integer_type(123.334));
        // boolean
        CHECK(config(false).as_integer() == 0);
        CHECK(config(true).as_integer() == 1);
        // null
        CHECK_THROWS_AS(config(nullptr).as_integer(), configor_type_error);
        // array
        CHECK_THROWS_AS(config::array({}).as_integer(), configor_type_error);
        // object
        CHECK_THROWS_AS(config::object({}).as_integer(), configor_type_error);
    }

    SECTION("test_as_float")
    {
        // string
        CHECK_THROWS_AS(config("12").as_float(), configor_type_error);
        // integer
        CHECK(config(123).as_float() == config::float_type(123));
        // floating
        CHECK(config(123.334).as_float() == config::float_type(123.334));
        // boolean
        CHECK(config(false).as_float() == 0);
        CHECK(config(true).as_float() == 1);
        // null
        CHECK_THROWS_AS(config(nullptr).as_float(), configor_type_error);
        // array
        CHECK_THROWS_AS(config::array({}).as_float(), configor_type_error);
        // object
        CHECK_THROWS_AS(config::object({}).as_float(), configor_type_error);
    }

    SECTION("test_as_string")
    {
        // string
        CHECK(config("string").as_string() == "string");
        // integer
        CHECK(config(123).as_string() == "123");
        // floating
        CHECK(config(0.01).as_string() == "0.01");
        // boolean
        CHECK_THROWS_AS(config(false).as_string(), configor_type_error);
        // null
        CHECK(config(nullptr).as_string() == "");
        // array
        CHECK_THROWS_AS(config::array({}).as_string(), configor_type_error);
        // object
        CHECK_THROWS_AS(config::object({}).as_string(), configor_type_error);
    }

    SECTION("test_method_size")
    {
        config c;
        // string
        c = "string";
        CHECK(c.size() == 1);
        // integer
        c = 100;
        CHECK(c.size() == 1);
        // floating
        c = 100.0;
        CHECK(c.size() == 1);
        // boolean
        c = true;
        CHECK(c.size() == 1);
        // null
        c = nullptr;
        CHECK(c.size() == 0);
        // array
        c = config::array({ 1, 2, 3 });
        CHECK(c.size() == 3);
        // object
        c = config::object({ { "1", 1 }, { "2", 2 } });
        CHECK(c.size() == 2);
    }

    SECTION("test_type")
    {
        using Catch::Matchers::Equals;

        config c;
        // string
        c = "string";
        CHECK(c.type() == config_value_type::string);
        CHECK_THAT(c.type_name(), Equals("string"));
        // integer
        c = 100;
        CHECK(c.type() == config_value_type::number_integer);
        CHECK_THAT(c.type_name(), Equals("integer"));
        // floating
        c = 100.0;
        CHECK(c.type() == config_value_type::number_float);
        CHECK_THAT(c.type_name(), Equals("float"));
        // boolean
        c = true;
        CHECK(c.type() == config_value_type::boolean);
        CHECK_THAT(c.type_name(), Equals("boolean"));
        // null
        c = nullptr;
        CHECK(c.type() == config_value_type::null);
        CHECK_THAT(c.type_name(), Equals("null"));
        // array
        c = config::array({ 1, 2, 3 });
        CHECK(c.type() == config_value_type::array);
        CHECK_THAT(c.type_name(), Equals("array"));
        // object
        c = config::object({ { "1", 1 }, { "2", 2 } });
        CHECK(c.type() == config_value_type::object);
        CHECK_THAT(c.type_name(), Equals("object"));
    }

    SECTION("test_method_clear")
    {
        config c;
        // string
        c = "string";
        CHECK_NOTHROW(c.clear());
        CHECK(c.get<std::string>() == "");
        // integer
        c = 100;
        CHECK_NOTHROW(c.clear());
        CHECK(c.get<int64_t>() == 0);
        // floating
        c = 100.0;
        CHECK_NOTHROW(c.clear());
        CHECK(c.get<double>() == 0);
        // boolean
        c = true;
        CHECK_NOTHROW(c.clear());
        CHECK_FALSE(c.get<bool>());
        // null
        c = nullptr;
        CHECK_NOTHROW(c.clear());
        CHECK(c.is_null());
        // array
        c = config::array({ 1, 2, 3 });
        CHECK_NOTHROW(c.clear());
        CHECK(c.size() == 0);
        // object
        c = config::object({ { "1", 1 }, { "2", 2 } });
        CHECK_NOTHROW(c.clear());
        CHECK(c.size() == 0);
    }

    SECTION("test_int64")
    {
        // issue 12
        int64_t max64 = std::numeric_limits<int64_t>::max();
        config  c     = max64;
        CHECK(c.get<int64_t>() == max64);
    }

    SECTION("test_config_value")
    {
        config c;

        CHECK_NOTHROW(c = config_value_type::null);
        CHECK(c.type() == config_value_type::null);

        CHECK_NOTHROW(c = config_value_type::boolean);
        CHECK(c.type() == config_value_type::boolean);
        CHECK_FALSE(c.get<bool>());

        CHECK_NOTHROW(c = config_value_type::number_integer);
        CHECK(c.type() == config_value_type::number_integer);
        CHECK(c.get<config::integer_type>() == 0);

        CHECK_NOTHROW(c = config_value_type::number_float);
        CHECK(c.type() == config_value_type::number_float);
        CHECK(c.get<config::float_type>() == 0.0);

        CHECK_NOTHROW(c = config_value_type::string);
        CHECK(c.type() == config_value_type::string);
        CHECK(c.get<config::string_type>() == std::string{});

        CHECK_NOTHROW(c = config_value_type::array);
        CHECK(c.type() == config_value_type::array);
        CHECK(c.get<config::array_type>() == config::array_type{});

        CHECK_NOTHROW(c = config_value_type::object);
        CHECK(c.type() == config_value_type::object);
        CHECK(c.get<config::object_type>() == config::object_type{});
    }
}

#include <fifo_map/fifo_map.hpp>

struct fifo_config_args : config_args
{
    template <class _Kty, class _Ty, class... _Args>
    using object_type = nlohmann::fifo_map<_Kty, _Ty>;
};

using fifo_config = basic_config<fifo_config_args>;

TEST_CASE("test_tpl_args")
{
    SECTION("test_tpl_args_map")
    {
        fifo_config c = fifo_config::object({ { "1", 1 }, { "2", 2 } });
        CHECK(c.size() == 2);
        CHECK(c["1"] == 1);
        CHECK(c["2"] == 2);

        std::string result;
        for (const auto& p : c)
        {
            result += p.as_string();
        }
        CHECK(result == "12");

        c.erase("1");
        c["1"] = 1;

        result.clear();
        for (const auto& p : c)
        {
            result += p.as_string();
        }
        CHECK(result == "21");
    }
}
