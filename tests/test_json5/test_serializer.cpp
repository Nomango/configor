// Copyright (c) 2019 Nomango

#include "common.h"

#include <cmath>    // std::acos
#include <iomanip>  // std::setw, std::fill, std::setprecision
#include <sstream>  // std::stringstream

class SerializerTest
{
protected:
    SerializerTest()
    {
        j = json5::object{
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Nomango" },
            { "nothing", nullptr },
            { "answer", json5::object{ { "everything", 42 } } },
            { "list", json5::array{ 1, 0, 2 } },
            { "object", json5::object{ { "currency", "USD" }, { "value", 42.99 } } },
        };
    }

    json5::value j;
};

TEST_CASE_METHOD(SerializerTest, "test_json5_dump")
{
    CHECK_NOTHROW(json5::dump(j));
    CHECK_NOTHROW(json5::dump(j, { json5::serializer::with_indent(4, ' ') }));

    // dump float
    // issue 7
    CHECK(json5::dump(json5::value(0.0)) == "0.0");
    CHECK(json5::dump(json5::value(1.0)) == "1.0");
    CHECK(wjson5::dump(wjson5::value(0.0)) == WIDE("0.0"));
    CHECK(wjson5::dump(wjson5::value(1.0)) == WIDE("1.0"));

    CHECK(json5::dump(json5::value(1.2)) == "1.2");
    CHECK(json5::dump(json5::value(1.23)) == "1.23");

    // dump empty object
    CHECK(json5::dump(json5::object{}) == "{}");

    // dump empty array
    CHECK(json5::dump(json5::array{}) == "[]");

    // dump boolean
    CHECK(json5::dump(json5::value(true)) == "true");
    CHECK(json5::dump(json5::value(false)) == "false");

    // dump control characters
    CHECK(json5::dump(json5::value("\t\r\n\b\f\"\\")) == "\"\\t\\r\\n\\b\\f\\\"\\\\\"");

    // invalid unicode
    CHECK_THROWS_AS(json5::dump(json5::value("\xC0")), configor_serialization_error);

    // test error policy
    error_handler_with<error_policy::strict> strict_handler{};
    CHECK_THROWS_AS(json5::dump(json5::value("\xC0"), { json5::serializer::with_error_handler(&strict_handler) }),
                    configor_serialization_error);

    error_handler_with<error_policy::ignore> ignore_handler{};
    CHECK_NOTHROW(json5::dump(json5::value("\xC0"), { json5::serializer::with_error_handler(&ignore_handler) }));

    error_handler_with<error_policy::record> record_handler{};
    CHECK_NOTHROW(json5::dump(json5::value("\xC0"), { json5::serializer::with_error_handler(&record_handler) }));
    CHECK_FALSE(record_handler.error.empty());
}

TEST_CASE_METHOD(SerializerTest, "test_json5_write_to_stream")
{
    std::stringstream ss;
    ss << json5::wrap(j);
    CHECK(ss.str() == json5::dump(j));

    ss.str("");
    ss << std::setw(4) << json5::wrap(j);
    CHECK(ss.str() == json5::dump(j, { json5::serializer::with_indent(4) }));

    ss.str("");
    ss << std::setw(2) << std::setfill('.') << json5::wrap(j);
    CHECK(ss.str() == json5::dump(j, { json5::serializer::with_indent(2, '.') }));
}

TEST_CASE_METHOD(SerializerTest, "test_json5_adapter")
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
        CHECK_NOTHROW(json5::dump(os, j));
        CHECK(output == json5::dump(j));
    }

    {
        output.clear();

        myadapter      ma{ output };
        oadapterstream os{ ma };

        os << 'h' << "ello,world";
        CHECK(output == "hello,world");
    }
}

TEST_CASE("test_json5_serializer")
{
    SECTION("test_numeric")
    {
        // dump integer
        CHECK(json5::dump(json5::value(0)) == "0");
        CHECK(json5::dump(json5::value(int32_t(2147483647))) == "2147483647");
        CHECK(json5::dump(json5::value(int64_t(9223372036854775807))) == "9223372036854775807");

        CHECK(wjson5::dump(wjson5::value(0)) == WIDE("0"));
        CHECK(wjson5::dump(wjson5::value(int32_t(2147483647))) == WIDE("2147483647"));
        CHECK(wjson5::dump(wjson5::value(int64_t(9223372036854775807))) == WIDE("9223372036854775807"));

        // dump signed integer
        CHECK(json5::dump(json5::value(-0)) == "0");
        CHECK(json5::dump(json5::value(int32_t(-2147483647))) == "-2147483647");
        CHECK(json5::dump(json5::value(int64_t(-9223372036854775807))) == "-9223372036854775807");

        CHECK(wjson5::dump(wjson5::value(-0)) == WIDE("0"));
        CHECK(wjson5::dump(wjson5::value(int32_t(-2147483647))) == WIDE("-2147483647"));
        CHECK(wjson5::dump(wjson5::value(int64_t(-9223372036854775807))) == WIDE("-9223372036854775807"));
    }

    SECTION("test_dump_intend")
    {
        json5::value j;
        j[0] = json5::object({ { "num", 1 } });
        j[1] = true;

        CHECK(json5::dump(j) == "[{\"num\":1},true]");
        CHECK(json5::dump(j, { json5::serializer::with_indent(4) })
              == "[\n    {\n        \"num\": 1\n    },\n    true\n]");
        CHECK(json5::dump(j, { json5::serializer::with_indent(2, '$') }) == "[\n$${\n$$$$\"num\":$1\n$$},\n$$true\n]");
    }

    SECTION("test_dump_minimal_float")
    {
        // issue 11
        const double pi            = std::acos(-1.0);
        const double minimal_float = pi / 1000000.0;

        json5::value j = minimal_float;
        CHECK(json5::dump(j) == "3.14159e-06");
        CHECK(json5::dump(j, { json5::serializer::with_precision(6, std::ios_base::fmtflags{}) }) == "3.14159e-06");
        CHECK(json5::dump(j, { json5::serializer::with_precision(std::numeric_limits<double>::digits10 + 1,
                                                               std::ios_base::fmtflags{}) })
              == "3.141592653589793e-06");

        j = json5::parse(json5::dump(j));
        CHECK(j.get<double>() == Approx(minimal_float));
    }

    SECTION("test_float_precision")
    {
        const double pi = std::acos(-1.0);
        json5::value  j  = pi;

        CHECK(json5::dump(j, { json5::serializer::with_precision(4) }) == "3.1416");

        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << json5::wrap(j);
        CHECK(ss.str() == "3.1416");

        ss.str("");
        ss << std::fixed << std::setprecision(12) << json5::wrap(j);
        CHECK(ss.str() == "3.141592653590");
    }

    SECTION("test_wrap")
    {
        int         i = 1;
        json5::value j = i;

        std::stringstream s;
        s << json5::wrap(i);
        CHECK(s.str() == json5::dump(j));

        int i2 = 0;
        s >> json5::wrap(i2);
        CHECK(i2 == i);
    }
}
