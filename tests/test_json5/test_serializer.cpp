// Copyright (c) 2019 Nomango

#include "common.h"

#include <cmath>    // std::acos
#include <iomanip>  // std::setw, std::fill, std::setprecision
#include <sstream>  // std::stringstream

class Json5SerializationTest
{
protected:
    Json5SerializationTest()
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

    json5 j;
};

TEST_CASE_METHOD(Json5SerializationTest, "test_json5_dump")
{
    CHECK_NOTHROW(j.dump());
    CHECK_NOTHROW(j.dump(4, ' '));

    // dump float
    // issue 7
    CHECK(json5(0.0).dump() == "0.0");
    CHECK(json5(1.0).dump() == "1.0");
    CHECK(wjson5(0.0).dump() == WIDE("0.0"));
    CHECK(wjson5(1.0).dump() == WIDE("1.0"));
    CHECK(u16json5(0.0).dump() == U16("0.0"));
    CHECK(u16json5(1.0).dump() == U16("1.0"));
    CHECK(u32json5(0.0).dump() == U32("0.0"));
    CHECK(u32json5(1.0).dump() == U32("1.0"));

    CHECK(json5(1.2).dump() == "1.2");
    CHECK(json5(1.23).dump() == "1.23");

    // dump empty object
    CHECK(json5::object({}).dump() == "{}");

    // dump empty array
    CHECK(json5::array({}).dump() == "[]");

    // dump boolean
    CHECK(json5(true).dump() == "true");
    CHECK(json5(false).dump() == "false");

    // dump control characters
    CHECK(json5("\t\r\n\b\f\"\\").dump() == "\"\\t\\r\\n\\b\\f\\\"\\\\\"");

    // invalid unicode
    CHECK_THROWS_AS(json5("\xC0").dump(), configor_serialization_error);

    // test error policy
    error_handler_with<error_policy::strict> strict_handler{};
    CHECK_THROWS_AS(json5("\xC0").dump(&strict_handler), configor_serialization_error);

    error_handler_with<error_policy::ignore> ignore_handler{};
    CHECK_NOTHROW(json5("\xC0").dump(&ignore_handler));

    error_handler_with<error_policy::record> record_handler{};
    CHECK_NOTHROW(json5("\xC0").dump(&record_handler));
    CHECK_FALSE(record_handler.error.empty());
}

TEST_CASE_METHOD(Json5SerializationTest, "test_json5_write_to_stream")
{
    std::stringstream ss;
    ss << j;
    CHECK(ss.str() == j.dump());

    ss.str("");
    ss << std::setw(4) << j;
    CHECK(ss.str() == j.dump(4));

    ss.str("");
    ss << std::setw(2) << std::setfill('.') << j;
    CHECK(ss.str() == j.dump(2, '.'));
}

TEST_CASE_METHOD(Json5SerializationTest, "test_json5_adapter")
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
        CHECK_NOTHROW(j.dump(os));
        CHECK(output == j.dump());
    }

    {
        output.clear();

        myadapter      ma{ output };
        oadapterstream os{ ma };

        os << 'h' << "ello,world";
        CHECK(output == "hello,world");
    }
}

TEST_CASE("test_json5_serialization")
{
    SECTION("test_numeric")
    {
        // dump integer
        CHECK(json5(0).dump() == "0");
        CHECK(json5(int32_t(2147483647)).dump() == "2147483647");
        CHECK(json5(int64_t(9223372036854775807)).dump() == "9223372036854775807");

        CHECK(wjson5(0).dump() == WIDE("0"));
        CHECK(wjson5(int32_t(2147483647)).dump() == WIDE("2147483647"));
        CHECK(wjson5(int64_t(9223372036854775807)).dump() == WIDE("9223372036854775807"));

        CHECK(u16json5(0).dump() == U16("0"));
        CHECK(u16json5(int32_t(2147483647)).dump() == U16("2147483647"));
        CHECK(u16json5(int64_t(9223372036854775807)).dump() == U16("9223372036854775807"));

        CHECK(u32json5(0).dump() == U32("0"));
        CHECK(u32json5(int32_t(2147483647)).dump() == U32("2147483647"));
        CHECK(u32json5(int64_t(9223372036854775807)).dump() == U32("9223372036854775807"));

        // dump signed integer
        CHECK(json5(-0).dump() == "0");
        CHECK(json5(int32_t(-2147483647)).dump() == "-2147483647");
        CHECK(json5(int64_t(-9223372036854775807)).dump() == "-9223372036854775807");

        CHECK(wjson5(-0).dump() == WIDE("0"));
        CHECK(wjson5(int32_t(-2147483647)).dump() == WIDE("-2147483647"));
        CHECK(wjson5(int64_t(-9223372036854775807)).dump() == WIDE("-9223372036854775807"));

        CHECK(u16json5(-0).dump() == U16("0"));
        CHECK(u16json5(int32_t(-2147483647)).dump() == U16("-2147483647"));
        CHECK(u16json5(int64_t(-9223372036854775807)).dump() == U16("-9223372036854775807"));

        CHECK(u32json5(-0).dump() == U32("0"));
        CHECK(u32json5(int32_t(-2147483647)).dump() == U32("-2147483647"));
        CHECK(u32json5(int64_t(-9223372036854775807)).dump() == U32("-9223372036854775807"));
    }

    SECTION("test_dump_intend")
    {
        json5 j;
        j[0] = json5::object({ { "num", 1 } });
        j[1] = true;

        CHECK(j.dump() == "[{\"num\":1},true]");
        CHECK(j.dump(4) == "[\n    {\n        \"num\": 1\n    },\n    true\n]");
        CHECK(j.dump(2, '$') == "[\n$${\n$$$$\"num\":$1\n$$},\n$$true\n]");
    }

    SECTION("test_dump_minimal_float")
    {
        // issue 11
        const double pi            = std::acos(-1.0);
        const double minimal_float = pi / 1000000.0;

        json5 j = minimal_float;
        CHECK(j.dump() == "3.141592653589793e-06");

        json5::writer::args args;
        args.precision = 6;
        CHECK(j.dump(args) == "3.14159e-06");

        j = json5::parse(j.dump());
        CHECK(j.get<double>() == Approx(minimal_float));
    }

    SECTION("test_float_precision")
    {
        const double pi = std::acos(-1.0);
        json5        j  = pi;

        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << j;
        CHECK(ss.str() == "3.1416");

        ss.str("");
        ss << std::fixed << std::setprecision(12) << j;
        CHECK(ss.str() == "3.141592653590");
    }

    SECTION("test_wrap")
    {
        int   i = 1;
        json5 j = i;

        std::stringstream s;
        s << json5::wrap(i);
        CHECK(s.str() == j.dump());

        int i2 = 0;
        s >> json5::wrap(i2);
        CHECK(i2 == i);
    }
}
