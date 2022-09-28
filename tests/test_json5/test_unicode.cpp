// Copyright (c) 2021 Nomango

#include "common.h"

#include <sstream>  // std::wstringstream

TEST_CASE("test_json5_unicode")
{
    SECTION("test_dump_surrogate")
    {
        json5::value j = RAW_STR;
        CHECK(json5::dump(j) == QUOTE_STR);
        CHECK(json5::dump(j, { json5::serializer::with_unicode_escaping(false) }) == QUOTE_STR);
        CHECK(json5::dump(j, { json5::serializer::with_unicode_escaping(true) }) == ESCAPED_STR);

        wjson5::value wj = WIDE(RAW_STR);
        CHECK(wjson5::dump(wj) == WIDE(QUOTE_STR));
        CHECK(wjson5::dump(wj, { wjson5::serializer::with_unicode_escaping(false) }) == WIDE(QUOTE_STR));
        CHECK(wjson5::dump(wj, { wjson5::serializer::with_unicode_escaping(true) }) == WIDE(ESCAPED_STR));
    }

    SECTION("test_ignore_encoding")
    {
        {
            json5::value j;
            CHECK_NOTHROW(j = json5::parse(QUOTE_STR, { json5::parser::with_encoding<encoding::ignore>() }));
            CHECK(j.get<std::string>() == RAW_STR);
        }
        {
            json5::value j = RAW_STR;
            CHECK(json5::dump(j, { json5::serializer::with_encoding<encoding::ignore>() }) == QUOTE_STR);
        }
    }

    SECTION("test_parse_w")
    {
        auto j = wjson5::parse(L"{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }");
        CHECK(j[L"happy"].get<bool>());
        CHECK(j[L"pi"].get<double>() == Approx(3.141));
        CHECK(j[L"name"].get<std::wstring>() == L"中文测试");
    }
}

class WCharTest
{
protected:
    WCharTest()
    {
        j = wjson5::object{
            { WIDE("pi"), 3.141 },
            { WIDE("happy"), true },
            { WIDE("name"), WIDE("Nomango") },
            { WIDE("nothing"), nullptr },
            { WIDE("answer"), wjson5::object{ { WIDE("everything"), 42 } } },
            { WIDE("list"), wjson5::array{ 1, 0, 2 } },
            { WIDE("object"), wjson5::object{ { WIDE("currency"), WIDE("USD") }, { WIDE("value"), 42.99 } } },
        };
    }

    wjson5::value j;
};

TEST_CASE_METHOD(WCharTest, "test_json5_write_to_stream_w")
{
    std::wstringstream ss;
    ss << wjson5::wrap(j);
    CHECK(ss.str() == wjson5::dump(j));

    ss.str(L"");
    ss << std::setw(4) << wjson5::wrap(j);
    CHECK(ss.str() == wjson5::dump(j, { wjson5::serializer::with_indent(4) }));

    ss.str(L"");
    ss << std::setw(2) << std::setfill(L'.') << wjson5::wrap(j);
    CHECK(ss.str() == wjson5::dump(j, { wjson5::serializer::with_indent(2, L'.') }));
}
