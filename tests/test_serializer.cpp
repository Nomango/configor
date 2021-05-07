// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <cmath>  // std::acos
#include <cfloat>  // DBL_DIG
#include <sstream>  // std::stringstream
#include <iomanip>  // std::setw, std::fill

using namespace jsonxx;

class SerializerTest : public testing::Test
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
            }},
        };
    }

    json j;
};

TEST_F(SerializerTest, test_write_to_stream)
{
    std::stringstream ss;
    ss << j;
    ASSERT_EQ(ss.str(), j.dump());

    ss.str("");
    ss << std::setw(4) << j;
    ASSERT_EQ(ss.str(), j.dump(4));

    ss.str("");
    ss << std::setw(2) << std::setfill('.') << j;
    ASSERT_EQ(ss.str(), j.dump(2, '.'));
}

TEST_F(SerializerTest, test_dump)
{
    ASSERT_NO_THROW(std::string serialized_string = j.dump());
    ASSERT_NO_THROW(std::string serialized_string = j.dump(4, ' '));

    // issue 7
    ASSERT_EQ(json(0.0).dump(), "0.0");
    ASSERT_EQ(json(1.0).dump(), "1.0");
    ASSERT_EQ(json(1.2).dump(), "1.2");
    ASSERT_EQ(json(1.23).dump(), "1.23");
}

TEST(test_serializer, test_dump_escaped)
{
    // issue 8
    json j = "我是地球🌍";
    ASSERT_EQ(j.dump(), "\"我是地球🌍\"");
    ASSERT_EQ(j.dump(-1, ' ', false), "\"我是地球🌍\"");
    ASSERT_EQ(j.dump(-1, ' ', true), "\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"");
}

TEST(test_serializer, test_dump_intend)
{
    json j;
    j[0] = json::object({"num", 1});
    j[1] = true;

    ASSERT_EQ(j.dump(), "[{\"num\":1},true]");
    ASSERT_EQ(j.dump(4), "[\n    {\n        \"num\": 1\n    },\n    true\n]");
    ASSERT_EQ(j.dump(2, '$'), "[\n$${\n$$$$\"num\":$1\n$$},\n$$true\n]");
}

TEST(test_serializer, test_dump_minimal_float)
{
    // issue 11
    const double pi = std::acos(-1.0);
    const double minimal_float = pi / 1000000.0;

    json j = minimal_float;
    ASSERT_EQ(j.dump(), "3.1415926535897933e-06");

#define COMBINE(A, B) A##B
#define PRECISION(DIG) COMBINE(1e-, DIG)

    j = json::parse(j.dump());
    ASSERT_NEAR(j.as_float(), minimal_float, PRECISION(DBL_DIG));
}
