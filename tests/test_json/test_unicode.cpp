// Copyright (c) 2021 Nomango

#include "common.h"

#include <sstream>  // std::wstringstream

TEST_CASE("test_unicode")
{
    SECTION("test_numeric")
    {
#define TEST_INT 2147483647
#define TEST_FLOAT 1.25

        u16json u16j;
        CHECK_NOTHROW(u16j = u16json::parse(U16(STR(TEST_INT))));
        CHECK(u16j.get<int64_t>() == TEST_INT);
        CHECK(u16j.dump() == U16(STR(TEST_INT)));

        CHECK_NOTHROW(u16j = u16json::parse(U16(STR(TEST_FLOAT))));
        CHECK(u16j.get<double>() == TEST_FLOAT);
        CHECK(u16j.dump() == U16(STR(TEST_FLOAT)));

        u32json u32j;
        CHECK_NOTHROW(u32j = u32json::parse(U32(STR(TEST_INT))));
        CHECK(u32j.get<int64_t>() == TEST_INT);
        CHECK(u32j.dump() == U32(STR(TEST_INT)));

        CHECK_NOTHROW(u32j = u32json::parse(U32(STR(TEST_FLOAT))));
        CHECK(u32j.get<double>() == TEST_FLOAT);
        CHECK(u32j.dump() == U32(STR(TEST_FLOAT)));
    }

    SECTION("test_parse_surrogate")
    {
        json j;
        CHECK_NOTHROW(j = json::parse(ESCAPED_STR));
        CHECK(j.get<std::string>() == RAW_STR);

        wjson wj;
        CHECK_NOTHROW(wj = wjson::parse(WIDE(ESCAPED_STR)));
        CHECK(wj.get<std::wstring>() == WIDE(RAW_STR));

        u16json u16j;
        CHECK_NOTHROW(u16j = u16json::parse(U16(ESCAPED_STR)));
        CHECK(u16j.get<std::u16string>() == U16(RAW_STR));

        u32json u32j;
        CHECK_NOTHROW(u32j = u32json::parse(U32(ESCAPED_STR)));
        CHECK(u32j.get<std::u32string>() == U32(RAW_STR));
    }

    SECTION("test_dump_surrogate")
    {
        json j = RAW_STR;
        CHECK(j.dump() == QUOTE_STR);
        CHECK(j.dump(-1, ' ', false) == QUOTE_STR);
        CHECK(j.dump(-1, ' ', true) == ESCAPED_STR);

        wjson wj = WIDE(RAW_STR);
        CHECK(wj.dump() == WIDE(QUOTE_STR));
        CHECK(wj.dump(-1, L' ', false) == WIDE(QUOTE_STR));
        CHECK(wj.dump(-1, L' ', true) == WIDE(ESCAPED_STR));

        u16json u16j = U16(RAW_STR);
        CHECK(u16j.dump() == U16(QUOTE_STR));
        CHECK(u16j.dump(-1, u' ', false) == U16(QUOTE_STR));
        CHECK(u16j.dump(-1, u' ', true) == U16(ESCAPED_STR));

        u32json u32j = U32(RAW_STR);
        CHECK(u32j.dump() == U32(QUOTE_STR));
        CHECK(u32j.dump(-1, U' ', false) == U32(QUOTE_STR));
        CHECK(u32j.dump(-1, U' ', true) == U32(ESCAPED_STR));
    }

    SECTION("test_ignore_encoding")
    {
        {
            json j;
            CHECK_NOTHROW(j = json::parse<encoding::ignore>(QUOTE_STR));
            CHECK(j.get<std::string>() == RAW_STR);
        }
        {
            json j = RAW_STR;
            CHECK(j.dump<encoding::ignore>() == QUOTE_STR);
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
        j = {
            { WIDE("pi"), 3.141 },
            { WIDE("happy"), true },
            { WIDE("name"), WIDE("Nomango") },
            { WIDE("nothing"), nullptr },
            { WIDE("answer"), { { WIDE("everything"), 42 } } },
            { WIDE("list"), { 1, 0, 2 } },
            { WIDE("object"), { { WIDE("currency"), WIDE("USD") }, { WIDE("value"), 42.99 } } },
        };
    }

    wjson j;
};

TEST_CASE_METHOD(WCharTest, "test_write_to_stream_w")
{
    std::wstringstream ss;
    ss << j;
    CHECK(ss.str() == j.dump());

    ss.str(L"");
    ss << std::setw(4) << j;
    CHECK(ss.str() == j.dump(4));

    ss.str(L"");
    ss << std::setw(2) << std::setfill(L'.') << j;
    CHECK(ss.str() == j.dump(2, L'.'));
}
