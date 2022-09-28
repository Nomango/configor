// Copyright (c) 2019 Nomango

#include "common.h"

class BasicConfigTest
{
protected:
    BasicConfigTest()
    {
        c = make_object<value>({ { "pi", 3.141 },
                                 { "happy", true },
                                 { "name", "Nomango" },
                                 { "chinese", "中文测试" },
                                 { "nothing", nullptr },
                                 { "list", make_array<value>({ 1, 0, 2 }) },
                                 { "object", make_object<value>({
                                                 { "currency", "USD" },
                                                 { "money", 42.99 },
                                             }) },
                                 { "single_object", make_object<value>({
                                                        { "number", 123 },
                                                    }) } });
    }

    value c;
};

TEST_CASE_METHOD(BasicConfigTest, "test_type")
{
    const auto& c = this->c;
    CHECK(c.is_object());
    CHECK(c["pi"].is_floating());
    CHECK(c["happy"].is_bool());
    CHECK(c["name"].is_string());
    CHECK(c["nothing"].is_null());
    CHECK(c["list"].is_array());
    CHECK(c["object"].is_object());
    CHECK(c["object"]["currency"].is_string());
    CHECK(c["object"]["money"].is_floating());
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

    CHECK(*c["list"].get<const value::array_type*>() == c["list"]);
    CHECK_NOTHROW(*c["list"].get<value::array_type*>() = value::array_type{ 5 });
    CHECK(c["list"].get<value::array_type>() == value::array_type{ 5 });

    CHECK(*c["single_object"].get<const value::object_type*>() == c["single_object"]);
    CHECK_NOTHROW(*c["single_object"].get<value::object_type*>() = value::object_type{});
    CHECK(c["single_object"].get<value::object_type>() == value::object_type{});
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

    CHECK(c["list"].get<const value::array_type&>() == c["list"]);
    CHECK_NOTHROW(c["list"].get<value::array_type&>() = value::array_type{ 5 });
    CHECK(c["list"].get<value::array_type>() == value::array_type{ 5 });

    CHECK(c["single_object"].get<const value::object_type&>() == c["single_object"]);
    CHECK_NOTHROW(c["single_object"].get<value::object_type&>() = value::object_type{});
    CHECK(c["single_object"].get<value::object_type>() == value::object_type{});
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
        auto  i = INT_TYPE(123);                               \
        value c = i;                                           \
        CHECK(c.is_integer());                                 \
        CHECK_FALSE(c.is_floating());                          \
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
        auto  i = FLOAT_TYPE(123.0);                            \
        value c = i;                                            \
        CHECK_FALSE(c.is_integer());                            \
        CHECK(c.is_floating());                                 \
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

    CHECK(value(1.0) == 1);
    CHECK(value(1) == 1.0);
    CHECK(value(1.0) == 1.0f);
    CHECK(value(1.0f) == 1.0);
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
        CHECK_NOTHROW(make_object<value>({ { "user", make_object<value>({ { "id", 1 }, { "name", "Nomango" } }) } }));

        // not an object
        // CANNOT COMPILE
        // CHECK_THROWS_AS(make_object<value>({ { "1", 1 }, { "" } }), configor_type_error);
        // CHECK_THROWS_AS(make_object<value>({ { 1, "" } }), configor_type_error);

        value c  = make_object<value>({ { "user", make_object<value>({ { "id", 1 }, { "name", "Nomango" } }) } });
        value j2 = c;
        CHECK(c == j2);

        CHECK_THROWS_AS(c[0], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const value&>(c)[0], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const value&>(c)["missing"], std::out_of_range);
    }

    SECTION("test_array")
    {
        CHECK_NOTHROW(make_array<value>({ 1, 2, 3 }));

        value c;
        CHECK_NOTHROW(c = make_array<value>({ make_array<value>(
                          { "user", make_object<value>({ { "id", 1 }, { "name", "Nomango" } }) }) }));
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0].is_array());

        CHECK_THROWS_AS(c["test"], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const value&>(c)["test"], configor_invalid_key);
        CHECK_THROWS_AS(const_cast<const value&>(c)[1], std::out_of_range);
    }

    SECTION("test_method_size")
    {
        value c;
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
        c = make_array<value>({ 1, 2, 3 });
        CHECK(c.size() == 3);
        // object
        c = make_object<value>({ { "1", 1 }, { "2", 2 } });
        CHECK(c.size() == 2);
    }

    SECTION("test_type")
    {
        using Catch::Matchers::Equals;

        value c;
        // string
        c = "string";
        CHECK(c.type() == value::string);
        CHECK_THAT(to_string(c.type()), Equals("string"));
        // integer
        c = 100;
        CHECK(c.type() == value::integer);
        CHECK_THAT(to_string(c.type()), Equals("integer"));
        // floating
        c = 100.0;
        CHECK(c.type() == value::floating);
        CHECK_THAT(to_string(c.type()), Equals("float"));
        // boolean
        c = true;
        CHECK(c.type() == value::boolean);
        CHECK_THAT(to_string(c.type()), Equals("boolean"));
        // null
        c = nullptr;
        CHECK(c.type() == value::null);
        CHECK_THAT(to_string(c.type()), Equals("null"));
        // array
        c = make_array<value>({ 1, 2, 3 });
        CHECK(c.type() == value::array);
        CHECK_THAT(to_string(c.type()), Equals("array"));
        // object
        c = make_object<value>({ { "1", 1 }, { "2", 2 } });
        CHECK(c.type() == value::object);
        CHECK_THAT(to_string(c.type()), Equals("object"));
    }

    SECTION("test_method_clear")
    {
        value c;
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
        c = make_array<value>({ 1, 2, 3 });
        CHECK_NOTHROW(c.clear());
        CHECK(c.size() == 0);
        // object
        c = make_object<value>({ { "1", 1 }, { "2", 2 } });
        CHECK_NOTHROW(c.clear());
        CHECK(c.size() == 0);
    }

    SECTION("test_int64")
    {
        // issue 12
        int64_t max64 = std::numeric_limits<int64_t>::max();
        value   c     = max64;
        CHECK(c.get<int64_t>() == max64);
    }

    SECTION("test_config_value")
    {
        value c;

        CHECK_NOTHROW(c = value::null);
        CHECK(c.type() == value::null);

        CHECK_NOTHROW(c = value::boolean);
        CHECK(c.type() == value::boolean);
        CHECK_FALSE(c.get<bool>());

        CHECK_NOTHROW(c = value::integer);
        CHECK(c.type() == value::integer);
        CHECK(c.get<value::integer_type>() == 0);

        CHECK_NOTHROW(c = value::floating);
        CHECK(c.type() == value::floating);
        CHECK(c.get<value::float_type>() == 0.0);

        CHECK_NOTHROW(c = value::string);
        CHECK(c.type() == value::string);
        CHECK(c.get<value::string_type>() == std::string{});

        CHECK_NOTHROW(c = value::array);
        CHECK(c.type() == value::array);
        CHECK(c.get<value::array_type>() == value::array_type{});

        CHECK_NOTHROW(c = value::object);
        CHECK(c.type() == value::object);
        CHECK(c.get<value::object_type>() == value::object_type{});
    }
}

#include <fifo_map/fifo_map.hpp>

struct fifo_value_tplargs : value_tplargs
{
    template <class _Kty, class _Ty, class... _Args>
    using object_type = nlohmann::fifo_map<_Kty, _Ty>;
};

using fifo_value = basic_value<fifo_value_tplargs>;

TEST_CASE("test_tpl_args")
{
    SECTION("test_tpl_args_map")
    {
        fifo_value c = make_object<fifo_value>({ { "1", 1 }, { "2", 2 } });
        CHECK(c.size() == 2);
        CHECK(c["1"] == 1);
        CHECK(c["2"] == 2);

        std::string result;
        for (const auto& p : c)
        {
            result += std::to_string(p.get<int>());
        }
        CHECK(result == "12");

        c.erase("1");
        c["1"] = 1;

        result.clear();
        for (const auto& p : c)
        {
            result += std::to_string(p.get<int>());
        }
        CHECK(result == "21");
    }
}
