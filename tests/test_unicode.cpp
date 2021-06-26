// Copyright (c) 2021 Nomango

#include "common.h"

TEST(test_unicode, test_numeric)
{
#define TEST_INT 2147483647
#define TEST_FLOAT 1.25

    u16json u16j;
    ASSERT_NO_THROW(u16j = u16json::parse(U16(STR(TEST_INT))));
    ASSERT_EQ(u16j.get<int64_t>(), TEST_INT);
    ASSERT_EQ(u16j.dump(), U16(STR(TEST_INT)));

    ASSERT_NO_THROW(u16j = u16json::parse(U16(STR(TEST_FLOAT))));
    ASSERT_EQ(u16j.get<double>(), TEST_FLOAT);
    ASSERT_EQ(u16j.dump(), U16(STR(TEST_FLOAT)));

    u32json u32j;
    ASSERT_NO_THROW(u32j = u32json::parse(U32(STR(TEST_INT))));
    ASSERT_EQ(u32j.get<int64_t>(), TEST_INT);
    ASSERT_EQ(u32j.dump(), U32(STR(TEST_INT)));

    ASSERT_NO_THROW(u32j = u32json::parse(U32(STR(TEST_FLOAT))));
    ASSERT_EQ(u32j.get<double>(), TEST_FLOAT);
    ASSERT_EQ(u32j.dump(), U32(STR(TEST_FLOAT)));
}

TEST(test_unicode, test_parse_surrogate)
{
    json j;
    ASSERT_NO_THROW(j = json::parse(ESCAPED_STR));
    ASSERT_EQ(j.get<std::string>(), RAW_STR);

    wjson wj;
    ASSERT_NO_THROW(wj = wjson::parse(WIDE(ESCAPED_STR)));
    ASSERT_EQ(wj.get<std::wstring>(), WIDE(RAW_STR));

    u16json u16j;
    ASSERT_NO_THROW(u16j = u16json::parse(U16(ESCAPED_STR)));
    ASSERT_EQ(u16j.get<std::u16string>(), U16(RAW_STR));

    u32json u32j;
    ASSERT_NO_THROW(u32j = u32json::parse(U32(ESCAPED_STR)));
    ASSERT_EQ(u32j.get<std::u32string>(), U32(RAW_STR));
}

TEST(test_unicode, test_dump_surrogate)
{
    json j = RAW_STR;
    ASSERT_EQ(j.dump(), QUOTE_STR);
    ASSERT_EQ(j.dump(-1, ' ', false), QUOTE_STR);
    ASSERT_EQ(j.dump(-1, ' ', true), ESCAPED_STR);

    wjson wj = WIDE(RAW_STR);
    ASSERT_EQ(wj.dump(), WIDE(QUOTE_STR));
    ASSERT_EQ(wj.dump(-1, ' ', false), WIDE(QUOTE_STR));
    ASSERT_EQ(wj.dump(-1, ' ', true), WIDE(ESCAPED_STR));

    u16json u16j = U16(RAW_STR);
    ASSERT_EQ(u16j.dump(), U16(QUOTE_STR));
    ASSERT_EQ(u16j.dump(-1, ' ', false), U16(QUOTE_STR));
    ASSERT_EQ(u16j.dump(-1, ' ', true), U16(ESCAPED_STR));

    u32json u32j = U32(RAW_STR);
    ASSERT_EQ(u32j.dump(), U32(QUOTE_STR));
    ASSERT_EQ(u32j.dump(-1, ' ', false), U32(QUOTE_STR));
    ASSERT_EQ(u32j.dump(-1, ' ', true), U32(ESCAPED_STR));
}

class WCharTest : public testing::Test
{
protected:
    void SetUp() override
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

TEST_F(WCharTest, test_write_to_stream)
{
    std::wstringstream ss;
    ss << j;
    ASSERT_EQ(ss.str(), j.dump());

    ss.str(L"");
    ss << std::setw(4) << j;
    ASSERT_EQ(ss.str(), j.dump(4));

    ss.str(L"");
    ss << std::setw(2) << std::setfill(L'.') << j;
    ASSERT_EQ(ss.str(), j.dump(2, '.'));
}

TEST(test_unicode, test_parse_w)
{
    auto j = wjson::parse(L"{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }");
    ASSERT_EQ(j[L"happy"].get<bool>(), true);
    ASSERT_DOUBLE_EQ(j[L"pi"].get<double>(), 3.141);
    ASSERT_EQ(j[L"name"].get<std::wstring>(), L"中文测试");
}
