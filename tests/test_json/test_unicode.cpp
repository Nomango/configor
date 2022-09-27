// Copyright (c) 2021 Nomango

#include "common.h"

#include <sstream>  // std::wstringstream

TEST_CASE("test_unicode")
{
    SECTION("test_dump_surrogate")
    {
        json::value j = RAW_STR;
        CHECK(json::dump(j) == QUOTE_STR);
        CHECK(json::dump(j, { json::serializer::with_unicode_escaping(false) }) == QUOTE_STR);
        CHECK(json::dump(j, { json::serializer::with_unicode_escaping(true) }) == ESCAPED_STR);

        wjson::value wj = WIDE(RAW_STR);
        CHECK(wjson::dump(wj) == WIDE(QUOTE_STR));
        CHECK(wjson::dump(wj, { wjson::serializer::with_unicode_escaping(false) }) == WIDE(QUOTE_STR));
        CHECK(wjson::dump(wj, { wjson::serializer::with_unicode_escaping(true) }) == WIDE(ESCAPED_STR));
    }

    SECTION("test_ignore_encoding")
    {
        {
            json::value j;
            CHECK_NOTHROW(j = json::parse(QUOTE_STR, { json::parser::with_encoding<encoding::ignore>() }));
            CHECK(j.get<std::string>() == RAW_STR);
        }
        {
            json::value j = RAW_STR;
            CHECK(json::dump(j, { json::serializer::with_encoding<encoding::ignore>() }) == QUOTE_STR);
        }
    }

    SECTION("test_parse_w")
    {
        auto j = wjson::parse(L"{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }");
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
        j = wjson::object{
            { WIDE("pi"), 3.141 },
            { WIDE("happy"), true },
            { WIDE("name"), WIDE("Nomango") },
            { WIDE("nothing"), nullptr },
            { WIDE("answer"), wjson::object{ { WIDE("everything"), 42 } } },
            { WIDE("list"), wjson::array{ 1, 0, 2 } },
            { WIDE("object"), wjson::object{ { WIDE("currency"), WIDE("USD") }, { WIDE("value"), 42.99 } } },
        };
    }

    wjson::value j;
};

TEST_CASE_METHOD(WCharTest, "test_write_to_stream_w")
{
    std::wstringstream ss;
    ss << wjson::wrap(j);
    CHECK(ss.str() == wjson::dump(j));

    ss.str(L"");
    ss << std::setw(4) << wjson::wrap(j);
    CHECK(ss.str() == wjson::dump(j, { wjson::serializer::with_indent(4) }));

    ss.str(L"");
    ss << std::setw(2) << std::setfill(L'.') << wjson::wrap(j);
    CHECK(ss.str() == wjson::dump(j, { wjson::serializer::with_indent(2, L'.') }));
}
