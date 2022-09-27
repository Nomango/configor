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
        j = json::object{
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Nomango" },
            { "nothing", nullptr },
            { "answer", json::object{ { "everything", 42 } } },
            { "list", json::array{ 1, 0, 2 } },
            { "object", json::object{ { "currency", "USD" }, { "value", 42.99 } } },
        };
    }

    json::value j;
};

TEST_CASE_METHOD(SerializerTest, "test_dump")
{
    CHECK_NOTHROW(json::dump(j));
    CHECK_NOTHROW(json::dump(j, { json::serializer::with_indent(4, ' ') }));

    // dump float
    // issue 7
    CHECK(json::dump(json::value(0.0)) == "0.0");
    CHECK(json::dump(json::value(1.0)) == "1.0");
    CHECK(wjson::dump(wjson::value(0.0)) == WIDE("0.0"));
    CHECK(wjson::dump(wjson::value(1.0)) == WIDE("1.0"));

    CHECK(json::dump(json::value(1.2)) == "1.2");
    CHECK(json::dump(json::value(1.23)) == "1.23");

    // dump empty object
    CHECK(json::dump(json::object{}) == "{}");

    // dump empty array
    CHECK(json::dump(json::array{}) == "[]");

    // dump boolean
    CHECK(json::dump(json::value(true)) == "true");
    CHECK(json::dump(json::value(false)) == "false");

    // dump control characters
    CHECK(json::dump(json::value("\t\r\n\b\f\"\\")) == "\"\\t\\r\\n\\b\\f\\\"\\\\\"");

    // invalid unicode
    CHECK_THROWS_AS(json::dump(json::value("\xC0")), configor_serialization_error);

    // test error policy
    error_handler_with<error_policy::strict> strict_handler{};
    CHECK_THROWS_AS(json::dump(json::value("\xC0"), { json::serializer::with_error_handler(&strict_handler) }),
                    configor_serialization_error);

    error_handler_with<error_policy::ignore> ignore_handler{};
    CHECK_NOTHROW(json::dump(json::value("\xC0"), { json::serializer::with_error_handler(&ignore_handler) }));

    error_handler_with<error_policy::record> record_handler{};
    CHECK_NOTHROW(json::dump(json::value("\xC0"), { json::serializer::with_error_handler(&record_handler) }));
    CHECK_FALSE(record_handler.error.empty());
}

TEST_CASE_METHOD(SerializerTest, "test_write_to_stream")
{
    std::stringstream ss;
    ss << json::wrap(j);
    CHECK(ss.str() == json::dump(j));

    ss.str("");
    ss << std::setw(4) << json::wrap(j);
    CHECK(ss.str() == json::dump(j, { json::serializer::with_indent(4) }));

    ss.str("");
    ss << std::setw(2) << std::setfill('.') << json::wrap(j);
    CHECK(ss.str() == json::dump(j, { json::serializer::with_indent(2, '.') }));
}

TEST_CASE_METHOD(SerializerTest, "test_adapter")
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
        CHECK_NOTHROW(json::dump(os, j));
        CHECK(output == json::dump(j));
    }

    {
        output.clear();

        myadapter      ma{ output };
        oadapterstream os{ ma };

        os << 'h' << "ello,world";
        CHECK(output == "hello,world");
    }
}

TEST_CASE("test_serializer")
{
    SECTION("test_numeric")
    {
        // dump integer
        CHECK(json::dump(json::value(0)) == "0");
        CHECK(json::dump(json::value(int32_t(2147483647))) == "2147483647");
        CHECK(json::dump(json::value(int64_t(9223372036854775807))) == "9223372036854775807");

        CHECK(wjson::dump(wjson::value(0)) == WIDE("0"));
        CHECK(wjson::dump(wjson::value(int32_t(2147483647))) == WIDE("2147483647"));
        CHECK(wjson::dump(wjson::value(int64_t(9223372036854775807))) == WIDE("9223372036854775807"));

        // dump signed integer
        CHECK(json::dump(json::value(-0)) == "0");
        CHECK(json::dump(json::value(int32_t(-2147483647))) == "-2147483647");
        CHECK(json::dump(json::value(int64_t(-9223372036854775807))) == "-9223372036854775807");

        CHECK(wjson::dump(wjson::value(-0)) == WIDE("0"));
        CHECK(wjson::dump(wjson::value(int32_t(-2147483647))) == WIDE("-2147483647"));
        CHECK(wjson::dump(wjson::value(int64_t(-9223372036854775807))) == WIDE("-9223372036854775807"));
    }

    SECTION("test_dump_intend")
    {
        json::value j;
        j[0] = json::object({ { "num", 1 } });
        j[1] = true;

        CHECK(json::dump(j) == "[{\"num\":1},true]");
        CHECK(json::dump(j, { json::serializer::with_indent(4) })
              == "[\n    {\n        \"num\": 1\n    },\n    true\n]");
        CHECK(json::dump(j, { json::serializer::with_indent(2, '$') }) == "[\n$${\n$$$$\"num\":$1\n$$},\n$$true\n]");
    }

    SECTION("test_dump_minimal_float")
    {
        // issue 11
        const double pi            = std::acos(-1.0);
        const double minimal_float = pi / 1000000.0;

        json::value j = minimal_float;
        CHECK(json::dump(j) == "3.14159e-06");
        CHECK(json::dump(j, { json::serializer::with_precision(6, std::ios_base::fmtflags{}) }) == "3.14159e-06");
        CHECK(json::dump(j, { json::serializer::with_precision(std::numeric_limits<double>::digits10 + 1,
                                                               std::ios_base::fmtflags{}) })
              == "3.141592653589793e-06");

        j = json::parse(json::dump(j));
        CHECK(j.get<double>() == Approx(minimal_float));
    }

    SECTION("test_float_precision")
    {
        const double pi = std::acos(-1.0);
        json::value  j  = pi;

        CHECK(json::dump(j, { json::serializer::with_precision(4) }) == "3.1416");

        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << json::wrap(j);
        CHECK(ss.str() == "3.1416");

        ss.str("");
        ss << std::fixed << std::setprecision(12) << json::wrap(j);
        CHECK(ss.str() == "3.141592653590");
    }

    SECTION("test_wrap")
    {
        int         i = 1;
        json::value j = i;

        std::stringstream s;
        s << json::wrap(i);
        CHECK(s.str() == json::dump(j));

        int i2 = 0;
        s >> json::wrap(i2);
        CHECK(i2 == i);
    }
}
