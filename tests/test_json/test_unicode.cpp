// Copyright (c) 2021 Nomango

#include "common.h"

#include <sstream>  // std::wstringstream

TEST_CASE("test_unicode")
{
    SECTION("test_numeric")
    {
#define TEST_INT 2147483647
#define TEST_FLOAT 1.25

        u16json::value u16j;
        CHECK_NOTHROW(u16j = u16json::parse(U16(STR(TEST_INT))));
        CHECK(u16j.get<int64_t>() == TEST_INT);
        CHECK(u16json::dump(u16j) == U16(STR(TEST_INT)));

        CHECK_NOTHROW(u16j = u16json::parse(U16(STR(TEST_FLOAT))));
        CHECK(u16j.get<double>() == TEST_FLOAT);
        CHECK(u16json::dump(u16j) == U16(STR(TEST_FLOAT)));

        u32json::value u32j;
        CHECK_NOTHROW(u32j = u32json::parse(U32(STR(TEST_INT))));
        CHECK(u32j.get<int64_t>() == TEST_INT);
        CHECK(u32json::dump(u32j) == U32(STR(TEST_INT)));

        CHECK_NOTHROW(u32j = u32json::parse(U32(STR(TEST_FLOAT))));
        CHECK(u32j.get<double>() == TEST_FLOAT);
        CHECK(u32json::dump(u32j) == U32(STR(TEST_FLOAT)));
    }

    SECTION("test_parse_surrogate")
    {
        json::value j;
        CHECK_NOTHROW(j = json::parse(ESCAPED_STR));
        CHECK(j.get<std::string>() == RAW_STR);

        wjson::value wj;
        CHECK_NOTHROW(wj = wjson::parse(WIDE(ESCAPED_STR)));
        CHECK(wj.get<std::wstring>() == WIDE(RAW_STR));

        u16json::value u16j;
        CHECK_NOTHROW(u16j = u16json::parse(U16(ESCAPED_STR)));
        CHECK(u16j.get<std::u16string>() == U16(RAW_STR));

        u32json::value u32j;
        CHECK_NOTHROW(u32j = u32json::parse(U32(ESCAPED_STR)));
        CHECK(u32j.get<std::u32string>() == U32(RAW_STR));
    }

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

        u16json::value u16j = U16(RAW_STR);
        CHECK(u16json::dump(u16j) == U16(QUOTE_STR));
        CHECK(u16json::dump(u16j, { u16json::serializer::with_unicode_escaping(false) }) == U16(QUOTE_STR));
        CHECK(u16json::dump(u16j, { u16json::serializer::with_unicode_escaping(true) }) == U16(ESCAPED_STR));

        u32json::value u32j = U32(RAW_STR);
        CHECK(u32json::dump(u32j) == U32(QUOTE_STR));
        CHECK(u32json::dump(u32j, { u32json::serializer::with_unicode_escaping(false) }) == U32(QUOTE_STR));
        CHECK(u32json::dump(u32j, { u32json::serializer::with_unicode_escaping(true) }) == U32(ESCAPED_STR));
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
