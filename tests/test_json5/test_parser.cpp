// Copyright (c) 2019 Nomango

#include "common.h"

#include <array>
#include <fstream>
#include <functional>

TEST_CASE("test_json5_parser")
{
    SECTION("test_parse")
    {
        json5::value j;

        // parse c-style string
        CHECK_NOTHROW(j = json5::parse("{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }"));
        CHECK(j["happy"].get<bool>());
        CHECK(j["pi"].get<double>() == Approx(3.141));
        CHECK(j["name"].get<std::string>() == "中文测试");

        // parse string
        CHECK_NOTHROW(j = json5::parse(std::string("{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }")));
        CHECK(j["happy"].get<bool>());
        CHECK(j["pi"].get<double>() == Approx(3.141));
        CHECK(j["name"].get<std::string>() == "中文测试");

        // parse empty object
        // issue 4
        CHECK_NOTHROW(j = json5::parse("{}"));
        CHECK((j.is_object() && j.empty()));

        // parse empty array
        CHECK_NOTHROW(j = json5::parse("[]"));
        CHECK((j.is_array() && j.empty()));

        // parse integer
        CHECK(json5::parse("0").get<int>() == 0);
        CHECK(json5::parse("2147483647").get<int32_t>() == int32_t(2147483647));
        CHECK(json5::parse("9223372036854775807").get<int64_t>() == int64_t(9223372036854775807));

        // parse signed integer
        CHECK(json5::parse("+0").get<int>() == 0);
        CHECK(json5::parse("+2147483647").get<int32_t>() == int32_t(2147483647));
        CHECK(json5::parse("+9223372036854775807").get<int64_t>() == int64_t(9223372036854775807));
        CHECK(json5::parse("-0").get<int>() == 0);
        CHECK(json5::parse("-2147483647").get<int32_t>() == int32_t(-2147483647));
        CHECK(json5::parse("-9223372036854775807").get<int64_t>() == int64_t(-9223372036854775807));

        // parse float
        CHECK(json5::parse("0.25").get<double>() == Approx(0.25));
        CHECK(json5::parse("1.25").get<double>() == Approx(1.25));
        CHECK(json5::parse("1.125e2").get<double>() == Approx(112.5));
        CHECK(json5::parse("0.125e2").get<double>() == Approx(12.5));
        CHECK(json5::parse("112.5e-2").get<double>() == Approx(1.125));
        CHECK(json5::parse("12.5e-2").get<double>() == Approx(0.125));

        // parse signed float
        CHECK(json5::parse("+0.25").get<double>() == Approx(0.25));
        CHECK(json5::parse("+1.25").get<double>() == Approx(1.25));
        CHECK(json5::parse("+1.125e2").get<double>() == Approx(112.5));
        CHECK(json5::parse("+0.125e2").get<double>() == Approx(12.5));
        CHECK(json5::parse("+112.5e-2").get<double>() == Approx(1.125));
        CHECK(json5::parse("+12.5e-2").get<double>() == Approx(0.125));

        CHECK(json5::parse("-0.25").get<double>() == Approx(-0.25));
        CHECK(json5::parse("-1.25").get<double>() == Approx(-1.25));
        CHECK(json5::parse("-1.125e2").get<double>() == Approx(-112.5));
        CHECK(json5::parse("-0.125e2").get<double>() == Approx(-12.5));
        CHECK(json5::parse("-112.5e-2").get<double>() == Approx(-1.125));
        CHECK(json5::parse("-12.5e-2").get<double>() == Approx(-0.125));
    }

    SECTION("test_parse_error")
    {
        // unexpected character
        CHECK_THROWS_AS(json5::parse("()"), configor_deserialization_error);

        // invalid literal
        CHECK_THROWS_AS(json5::parse("trux"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("falsx"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("nulx"), configor_deserialization_error);

        // unexpected end of string
        CHECK_THROWS_AS(json5::parse("\""), configor_deserialization_error);

        // parse controle characters
        CHECK_THROWS_AS(json5::parse("\"\t\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\r\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\n\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\b\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\f\""), configor_deserialization_error);

        // invalid escaped character
        CHECK_THROWS_AS(json5::parse("\"\\x\""), configor_deserialization_error);

        // invalid surrogate
        CHECK_THROWS_AS(json5::parse("\"\\uD8\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\\uD800\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\\uD800\\uD800\""), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("\"\\uD800\\x\""), configor_deserialization_error);

        // invalid float
        CHECK_THROWS_AS(json5::parse("0.x"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("0e1"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("1ex"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("1e0"), configor_deserialization_error);

        // unexpect end
        CHECK_THROWS_AS(json5::parse("\\"), configor_deserialization_error);

        // unexpect token
        CHECK_THROWS_AS(json5::parse("]"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("}"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("{]"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("[}"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("{}{"), configor_deserialization_error);
    }

    SECTION("test_error_policy")
    {
        error_handler_with<error_policy::strict> strict_handler{};
        CHECK_THROWS_AS(json5::parse("\f", { json5::parser::with_error_handler(&strict_handler) }),
                        configor_deserialization_error);

        error_handler_with<error_policy::ignore> ignore_handler{};
        CHECK_NOTHROW(json5::parse("\f", { json5::parser::with_error_handler(&ignore_handler) }));

        error_handler_with<error_policy::record> record_handler{};
        CHECK_NOTHROW(json5::parse("\f", { json5::parser::with_error_handler(&record_handler) }));
        CHECK_FALSE(record_handler.error.empty());
    }

    SECTION("test_comment")
    {
        auto j = json5::parse(R"(// some comments
        /* some comments */
        {
            // some comments
            /* some comments */ "happy": true,  /* some comments */
            // "pi": 1,
            "pi": 3.141, // some comments
            // "pi": 2,
            /*
            some comments
            "pi": 3,
            */"name": "中文测试"
        }// some comments)");
        CHECK(j["happy"].get<bool>());
        CHECK(j["pi"].get<double>() == Approx(3.141));
        CHECK(j["name"].get<std::string>() == "中文测试");

        CHECK_NOTHROW(json5::parse("{/**/}"));
        CHECK_NOTHROW(json5::parse("{//\n}"));
        CHECK_THROWS_AS(json5::parse("{/x\n}"), configor_deserialization_error);

        CHECK_THROWS_AS(json5::parse("/* aaaa"), configor_deserialization_error);
        CHECK_THROWS_AS(json5::parse("/* aaaa *"), configor_deserialization_error);
    }

    SECTION("test_parse_surrogate")
    {
        // issue 8
        auto j = json5::parse("\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"");
        CHECK(j.get<std::string>() == "我是地球🌍");
    }

    SECTION("test_read_from_file")
    {
        std::array<std::string, 5> files = {
            "tests/data/json.org/1.json", "tests/data/json.org/2.json", "tests/data/json.org/3.json",
            "tests/data/json.org/4.json", "tests/data/json.org/5.json",
        };

        std::function<void(json5::value&)> tests[] = {
            [](json5::value& j)
            {
                // test 1
                auto list = j["glossary"]["GlossDiv"]["GlossList"]["GlossEntry"]["GlossDef"]["GlossSeeAlso"];
                CHECK(list[0].get<std::string>() == "GML");
                CHECK(list[1].get<std::string>() == "XML");
            },
            [](json5::value& j)
            {
                // test 2
                CHECK(j["menu"]["popup"]["menuitem"][0]["onclick"].get<std::string>() == "CreateNewDoc()");
            },
            [](json5::value& j)
            {
                // test 3
            },
            [](json5::value& j)
            {
                // test 4
            },
            [](json5::value& j)
            {
                // test 5
                CHECK(j["menu"]["items"][2].is_null());
                CHECK(j["menu"]["items"][3]["id"].get<std::string>() == "ZoomIn");
            },
        };

        for (size_t i = 0; i < files.size(); i++)
        {
            // read a json5 file
            std::ifstream ifs(files[i]);

            json5::value j;
            CHECK_NOTHROW((ifs >> json5::wrap(j)));

            // run tests
            tests[i](j);
        }
    }

    SECTION("test_adapter")
    {
        struct myadapter : public iadapter
        {
            myadapter(const std::string& str)
                : str_(str)
                , idx_(0)
            {
            }

            virtual char read() override
            {
                if (idx_ >= str_.size())
                    return '\0';
                // return std::char_traits<char>::eof();
                return str_[idx_++];
            }

        private:
            const std::string& str_;
            size_t             idx_;
        };

        std::string input = "{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }";

        {
            myadapter      ma{ input };
            iadapterstream is{ ma };
            CHECK(json5::parse(is) == json5::parse(input));
        }

        {
            using Catch::Matchers::Equals;

            myadapter      ma{ input };
            iadapterstream is{ ma };
            CHECK(is.get() == '{');
            CHECK(is.peek() == ' ');
            CHECK(is.get() == ' ');
            CHECK(is.peek() == '\"');
            CHECK(is.get() == '\"');

            char str[6] = {};
            is.get(str, 6);
            CHECK_THAT(str, Equals("happy"));
            CHECK(is.good());

            CHECK(is.get() == '\"');
            CHECK(is.get() == ':');
            CHECK(is.get() == ' ');

            is.get(str, 5);
            CHECK_THAT(str, Equals("true"));
            CHECK(is.good());
        }
    }
}
