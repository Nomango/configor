// Copyright (c) 2019 Nomango

#include "common.h"

#include <cfloat>   // DBL_DIG
#include <cmath>    // std::acos
#include <iomanip>  // std::setw, std::fill
#include <sstream>  // std::stringstream

class SerializerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = {
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Nomango" },
            { "nothing", nullptr },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } },
        };
    }

    json j;
};

TEST_F(SerializerTest, test_dump)
{
    ASSERT_NO_THROW(std::string serialized_string = j.dump());
    ASSERT_NO_THROW(std::string serialized_string = j.dump(4, ' '));

    // dump float
    // issue 7
    ASSERT_EQ(json(0.0).dump(), "0.0");
    ASSERT_EQ(json(1.0).dump(), "1.0");
    ASSERT_EQ(wjson(0.0).dump(), WIDE("0.0"));
    ASSERT_EQ(wjson(1.0).dump(), WIDE("1.0"));
    ASSERT_EQ(u16json(0.0).dump(), U16("0.0"));
    ASSERT_EQ(u16json(1.0).dump(), U16("1.0"));
    ASSERT_EQ(u32json(0.0).dump(), U32("0.0"));
    ASSERT_EQ(u32json(1.0).dump(), U32("1.0"));

    ASSERT_EQ(json(1.2).dump(), "1.2");
    ASSERT_EQ(json(1.23).dump(), "1.23");

    // dump empty object
    ASSERT_EQ(json::object({}).dump(), "{}");

    // dump empty array
    ASSERT_EQ(json::array({}).dump(), "[]");

    // dump boolean
    ASSERT_EQ(json(true).dump(), "true");
    ASSERT_EQ(json(false).dump(), "false");

    // dump control characters
    ASSERT_EQ(json("\t\r\n\b\f\"\\").dump(), "\"\\t\\r\\n\\b\\f\\\"\\\\\"");

    // invalid unicode
    ASSERT_THROW(json("\xC0").dump(), json_serialization_error);

    // test error policy
    error_handler_with<error_policy::strict> strict_handler{};
    ASSERT_THROW(json("\xC0").dump(json::dump_args{}, &strict_handler), json_serialization_error);

    error_handler_with<error_policy::ignore> ignore_handler{};
    ASSERT_NO_THROW(json("\xC0").dump(json::dump_args{}, &ignore_handler));

    error_handler_with<error_policy::record> record_handler{};
    ASSERT_NO_THROW(json("\xC0").dump(json::dump_args{}, &record_handler));
    ASSERT_FALSE(record_handler.error.empty());
}

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

TEST(test_serializer, test_numeric)
{
    // dump integer
    ASSERT_EQ(json(0).dump(), "0");
    ASSERT_EQ(json(int32_t(2147483647)).dump(), "2147483647");
    ASSERT_EQ(json(int64_t(9223372036854775807)).dump(), "9223372036854775807");

    ASSERT_EQ(wjson(0).dump(), WIDE("0"));
    ASSERT_EQ(wjson(int32_t(2147483647)).dump(), WIDE("2147483647"));
    ASSERT_EQ(wjson(int64_t(9223372036854775807)).dump(), WIDE("9223372036854775807"));

    ASSERT_EQ(u16json(0).dump(), U16("0"));
    ASSERT_EQ(u16json(int32_t(2147483647)).dump(), U16("2147483647"));
    ASSERT_EQ(u16json(int64_t(9223372036854775807)).dump(), U16("9223372036854775807"));

    ASSERT_EQ(u32json(0).dump(), U32("0"));
    ASSERT_EQ(u32json(int32_t(2147483647)).dump(), U32("2147483647"));
    ASSERT_EQ(u32json(int64_t(9223372036854775807)).dump(), U32("9223372036854775807"));

    // dump signed integer
    ASSERT_EQ(json(-0).dump(), "0");
    ASSERT_EQ(json(int32_t(-2147483647)).dump(), "-2147483647");
    ASSERT_EQ(json(int64_t(-9223372036854775807)).dump(), "-9223372036854775807");

    ASSERT_EQ(wjson(-0).dump(), WIDE("0"));
    ASSERT_EQ(wjson(int32_t(-2147483647)).dump(), WIDE("-2147483647"));
    ASSERT_EQ(wjson(int64_t(-9223372036854775807)).dump(), WIDE("-9223372036854775807"));

    ASSERT_EQ(u16json(-0).dump(), U16("0"));
    ASSERT_EQ(u16json(int32_t(-2147483647)).dump(), U16("-2147483647"));
    ASSERT_EQ(u16json(int64_t(-9223372036854775807)).dump(), U16("-9223372036854775807"));

    ASSERT_EQ(u32json(-0).dump(), U32("0"));
    ASSERT_EQ(u32json(int32_t(-2147483647)).dump(), U32("-2147483647"));
    ASSERT_EQ(u32json(int64_t(-9223372036854775807)).dump(), U32("-9223372036854775807"));
}

TEST(test_serializer, test_dump_intend)
{
    json j;
    j[0] = json::object({ { "num", 1 } });
    j[1] = true;

    ASSERT_EQ(j.dump(), "[{\"num\":1},true]");
    ASSERT_EQ(j.dump(4), "[\n    {\n        \"num\": 1\n    },\n    true\n]");
    ASSERT_EQ(j.dump(2, '$'), "[\n$${\n$$$$\"num\":$1\n$$},\n$$true\n]");
}

TEST(test_serializer, test_dump_minimal_float)
{
    // issue 11
    const double pi            = std::acos(-1.0);
    const double minimal_float = pi / 1000000.0;

    json j = minimal_float;
    ASSERT_EQ(j.dump(), "3.141592653589793e-06");

    json::dump_args args;
    args.precision = 6;
    ASSERT_EQ(j.dump(args), "3.14159e-06");

#define COMBINE(A, B) A##B
#define PRECISION(DIG) COMBINE(1e-, DIG)

    j = json::parse(j.dump());
    ASSERT_NEAR(j.get<double>(), minimal_float, PRECISION(DBL_DIG));
}

TEST_F(SerializerTest, test_adapter)
{
    struct myadapter : public oadapter
    {
        myadapter(std::string& str)
            : str_(str)
        {
        }

        virtual void write(const char ch) override
        {
            str_.push_back(ch);
        }

    private:
        std::string& str_;
    };

    std::string output;
    {
        myadapter      ma{ output };
        oadapterstream os{ ma };
        ASSERT_NO_THROW(j.dump(os));
        ASSERT_EQ(output, j.dump());
    }

    {
        output.clear();

        myadapter      ma{ output };
        oadapterstream os{ ma };

        os << 'h' << "ello,world";
        ASSERT_EQ(output, "hello,world");
    }
}
