// Copyright (c) 2019 Nomango

#include "common.h"

#include <array>
#include <fstream>
#include <functional>

TEST_CASE("test_parser")
{
    SECTION("test_parse")
    {
        json j;

        // parse c-style string
        CHECK_NOTHROW(j = json::parse("{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }"));
        CHECK(j["happy"].get<bool>());
        CHECK(j["pi"].get<double>() == Approx(3.141));
        CHECK(j["name"].get<std::string>() == "中文测试");

        // parse string
        CHECK_NOTHROW(j = json::parse(std::string("{ \"happy\": true, \"pi\": 3.141, \"name\": \"中文测试\" }")));
        CHECK(j["happy"].get<bool>());
        CHECK(j["pi"].get<double>() == Approx(3.141));
        CHECK(j["name"].get<std::string>() == "中文测试");

        // parse empty object
        // issue 4
        CHECK_NOTHROW(j = json::parse("{}"));
        CHECK((j.is_object() && j.empty()));

        // parse empty array
        CHECK_NOTHROW(j = json::parse("[]"));
        CHECK((j.is_array() && j.empty()));

        // parse integer
        CHECK(json::parse("0").get<int>() == 0);
        CHECK(json::parse("2147483647").get<int32_t>() == int32_t(2147483647));
        CHECK(json::parse("9223372036854775807").get<int64_t>() == int64_t(9223372036854775807));

        // parse signed integer
        CHECK(json::parse("+0").get<int>() == 0);
        CHECK(json::parse("+2147483647").get<int32_t>() == int32_t(2147483647));
        CHECK(json::parse("+9223372036854775807").get<int64_t>() == int64_t(9223372036854775807));
        CHECK(json::parse("-0").get<int>() == 0);
        CHECK(json::parse("-2147483647").get<int32_t>() == int32_t(-2147483647));
        CHECK(json::parse("-9223372036854775807").get<int64_t>() == int64_t(-9223372036854775807));

        // parse float
        CHECK(json::parse("0.25").get<double>() == Approx(0.25));
        CHECK(json::parse("1.25").get<double>() == Approx(1.25));
        CHECK(json::parse("1.125e2").get<double>() == Approx(112.5));
        CHECK(json::parse("0.125e2").get<double>() == Approx(12.5));
        CHECK(json::parse("112.5e-2").get<double>() == Approx(1.125));
        CHECK(json::parse("12.5e-2").get<double>() == Approx(0.125));

        // parse signed float
        CHECK(json::parse("+0.25").get<double>() == Approx(0.25));
        CHECK(json::parse("+1.25").get<double>() == Approx(1.25));
        CHECK(json::parse("+1.125e2").get<double>() == Approx(112.5));
        CHECK(json::parse("+0.125e2").get<double>() == Approx(12.5));
        CHECK(json::parse("+112.5e-2").get<double>() == Approx(1.125));
        CHECK(json::parse("+12.5e-2").get<double>() == Approx(0.125));

        CHECK(json::parse("-0.25").get<double>() == Approx(-0.25));
        CHECK(json::parse("-1.25").get<double>() == Approx(-1.25));
        CHECK(json::parse("-1.125e2").get<double>() == Approx(-112.5));
        CHECK(json::parse("-0.125e2").get<double>() == Approx(-12.5));
        CHECK(json::parse("-112.5e-2").get<double>() == Approx(-1.125));
        CHECK(json::parse("-12.5e-2").get<double>() == Approx(-0.125));
    }

    SECTION("test_parse_error")
    {
        // unexpected character
        CHECK_THROWS_AS(json::parse("()"), configor_deserialization_error);

        // invalid literal
        CHECK_THROWS_AS(json::parse("trux"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("falsx"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("nulx"), configor_deserialization_error);

        // unexpected end of string
        CHECK_THROWS_AS(json::parse("\""), configor_deserialization_error);

        // parse controle characters
        CHECK_THROWS_AS(json::parse("\"\t\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\r\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\n\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\b\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\f\""), configor_deserialization_error);

        // invalid escaped character
        CHECK_THROWS_AS(json::parse("\"\\x\""), configor_deserialization_error);

        // invalid surrogate
        CHECK_THROWS_AS(json::parse("\"\\uD8\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\\uD800\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\\uD800\\uD800\""), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("\"\\uD800\\x\""), configor_deserialization_error);

        // invalid float
        CHECK_THROWS_AS(json::parse("0.x"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("0e1"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("1ex"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("1e0"), configor_deserialization_error);

        // not allow comments
        {
            // TODO
            CHECK_NOTHROW(json::parse("{/**/}"));
            CHECK_NOTHROW(json::parse("{//\n}"));
            CHECK_THROWS_AS(json::parse("{/x\n}"), configor_deserialization_error);
        }

        // unexpect end
        CHECK_THROWS_AS(json::parse("\\"), configor_deserialization_error);

        // unexpect token
        CHECK_THROWS_AS(json::parse("]"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("}"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("{]"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("[}"), configor_deserialization_error);
        CHECK_THROWS_AS(json::parse("{}{"), configor_deserialization_error);
    }

    SECTION("test_error_policy")
    {
        error_handler_with<error_policy::strict> strict_handler{};
        CHECK_THROWS_AS(json::parse("\f", &strict_handler), configor_deserialization_error);

        error_handler_with<error_policy::ignore> ignore_handler{};
        CHECK_NOTHROW(json::parse("\f", &ignore_handler));

        error_handler_with<error_policy::record> record_handler{};
        CHECK_NOTHROW(json::parse("\f", &record_handler));
        CHECK_FALSE(record_handler.error.empty());
    }

    SECTION("test_comment")
    {
        // TODO
        auto j = json::parse(R"(// some comments
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
    }

    SECTION("test_parse_surrogate")
    {
        // issue 8
        auto j = json::parse("\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"");
        CHECK(j.get<std::string>() == "我是地球🌍");
    }

    SECTION("test_read_from_file")
    {
        std::array<std::string, 5> files = {
            "tests/data/json.org/1.json", "tests/data/json.org/2.json", "tests/data/json.org/3.json",
            "tests/data/json.org/4.json", "tests/data/json.org/5.json",
        };

        std::function<void(json&)> tests[] = {
            [](json& j)
            {
                // test 1
                auto list = j["glossary"]["GlossDiv"]["GlossList"]["GlossEntry"]["GlossDef"]["GlossSeeAlso"];
                CHECK(list[0].get<std::string>() == "GML");
                CHECK(list[1].get<std::string>() == "XML");
            },
            [](json& j)
            {
                // test 2
                CHECK(j["menu"]["popup"]["menuitem"][0]["onclick"].get<std::string>() == "CreateNewDoc()");
            },
            [](json& j)
            {
                // test 3
            },
            [](json& j)
            {
                // test 4
            },
            [](json& j)
            {
                // test 5
                CHECK(j["menu"]["items"][2].is_null());
                CHECK(j["menu"]["items"][3]["id"].get<std::string>() == "ZoomIn");
            },
        };

        for (size_t i = 0; i < files.size(); i++)
        {
            // read a json file
            std::ifstream ifs(files[i]);

            json j;
            CHECK_NOTHROW((ifs >> j));

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
            CHECK(json::parse(is) == json::parse(input));
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
