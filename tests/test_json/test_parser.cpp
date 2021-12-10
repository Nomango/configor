// Copyright (c) 2019 Nomango

#include "common.h"

#include <array>
#include <fstream>
#include <functional>

TEST_CASE("test_json_parser")
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
                json expect = {
                    { "glossary",
                      { { "title", "example glossary" },
                        { "GlossDiv",
                          { { "title", "S" },
                            { "GlossList",
                              { { "GlossEntry",
                                  {
                                      { "ID", "SGML" },
                                      { "SortAs", "SGML" },
                                      { "GlossTerm", "Standard Generalized Markup Language" },
                                      { "Acronym", "SGML" },
                                      { "Abbrev", "ISO 8879:1986" },
                                      {
                                          "GlossDef",
                                          { { "para", "A meta-markup language, used to create markup languages such "
                                                      "as DocBook." },
                                            { "GlossSeeAlso", json::array({ "GML", "XML" }) } },
                                      },
                                      { "GlossSee", "markup" },
                                  } } } } } } } }
                };
                CHECK(j == expect);
            },
            [](json& j)
            {
                json expect = { { "menu",
                                  { { "id", "file" },
                                    { "value", "File" },
                                    { "popup",
                                      { { "menuitem",
                                          { { { "value", "New" }, { "onclick", "CreateNewDoc()" } },
                                            { { "value", "Open" }, { "onclick", "OpenDoc()" } },
                                            { { "value", "Close" }, { "onclick", "CloseDoc()" } } } } } } } } };
                CHECK(j == expect);
            },
            [](json& j)
            {
                json expect = { { "widget",
                                  { { "debug", "on" },
                                    { "window",
                                      { { "title", "Sample Konfabulator Widget" },
                                        { "name", "main_window" },
                                        { "width", 500 },
                                        { "height", 500 } } },
                                    { "image",
                                      { { "src", "Images/Sun.png" },
                                        { "name", "sun1" },
                                        { "hOffset", 250 },
                                        { "vOffset", 250 },
                                        { "alignment", "center" } } },
                                    { "text",
                                      { { "data", "Click Here" },
                                        { "size", 36 },
                                        { "style", "bold" },
                                        { "name", "text1" },
                                        { "hOffset", 250 },
                                        { "vOffset", 100 },
                                        { "alignment", "center" },
                                        { "onMouseUp", "sun1.opacity = (sun1.opacity / 100) * 90;" } } } } } };
                CHECK(j == expect);
            },
            [](json& j)
            {
                json expect = {
                    { "web-app",
                      { { "servlet",
                          { { { "servlet-name", "cofaxCDS" },
                              { "servlet-class", "org.cofax.cds.CDSServlet" },
                              { "init-param",
                                { { "configGlossary:installationAt", "Philadelphia, PA" },
                                  { "configGlossary:adminEmail", "ksm@pobox.com" },
                                  { "configGlossary:poweredBy", "Cofax" },
                                  { "configGlossary:poweredByIcon", "/images/cofax.gif" },
                                  { "configGlossary:staticPath", "/content/static" },
                                  { "templateProcessorClass", "org.cofax.WysiwygTemplate" },
                                  { "templateLoaderClass", "org.cofax.FilesTemplateLoader" },
                                  { "templatePath", "templates" },
                                  { "templateOverridePath", "" },
                                  { "defaultListTemplate", "listTemplate.htm" },
                                  { "defaultFileTemplate", "articleTemplate.htm" },
                                  { "useJSP", false },
                                  { "jspListTemplate", "listTemplate.jsp" },
                                  { "jspFileTemplate", "articleTemplate.jsp" },
                                  { "cachePackageTagsTrack", 200 },
                                  { "cachePackageTagsStore", 200 },
                                  { "cachePackageTagsRefresh", 60 },
                                  { "cacheTemplatesTrack", 100 },
                                  { "cacheTemplatesStore", 50 },
                                  { "cacheTemplatesRefresh", 15 },
                                  { "cachePagesTrack", 200 },
                                  { "cachePagesStore", 100 },
                                  { "cachePagesRefresh", 10 },
                                  { "cachePagesDirtyRead", 10 },
                                  { "searchEngineListTemplate", "forSearchEnginesList.htm" },
                                  { "searchEngineFileTemplate", "forSearchEngines.htm" },
                                  { "searchEngineRobotsDb", "WEB-INF/robots.db" },
                                  { "useDataStore", true },
                                  { "dataStoreClass", "org.cofax.SqlDataStore" },
                                  { "redirectionClass", "org.cofax.SqlRedirection" },
                                  { "dataStoreName", "cofax" },
                                  { "dataStoreDriver", "com.microsoft.jdbc.sqlserver.SQLServerDriver" },
                                  { "dataStoreUrl", "jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon" },
                                  { "dataStoreUser", "sa" },
                                  { "dataStorePassword", "dataStoreTestQuery" },
                                  { "dataStoreTestQuery", "SET NOCOUNT ON;select test='test';" },
                                  { "dataStoreLogFile", "/usr/local/tomcat/logs/datastore.log" },
                                  { "dataStoreInitConns", 10 },
                                  { "dataStoreMaxConns", 100 },
                                  { "dataStoreConnUsageLimit", 100 },
                                  { "dataStoreLogLevel", "debug" },
                                  { "maxUrlLength", 500 } } } },
                            { { "servlet-name", "cofaxEmail" },
                              { "servlet-class", "org.cofax.cds.EmailServlet" },
                              { "init-param", { { "mailHost", "mail1" }, { "mailHostOverride", "mail2" } } } },
                            { { "servlet-name", "cofaxAdmin" }, { "servlet-class", "org.cofax.cds.AdminServlet" } },

                            { { "servlet-name", "fileServlet" }, { "servlet-class", "org.cofax.cds.FileServlet" } },
                            { { "servlet-name", "cofaxTools" },
                              { "servlet-class", "org.cofax.cms.CofaxToolsServlet" },
                              { "init-param",
                                { { "templatePath", "toolstemplates/" },
                                  { "log", 1 },
                                  { "logLocation", "/usr/local/tomcat/logs/CofaxTools.log" },
                                  { "logMaxSize", "" },
                                  { "dataLog", 1 },
                                  { "dataLogLocation", "/usr/local/tomcat/logs/dataLog.log" },
                                  { "dataLogMaxSize", "" },
                                  { "removePageCache", "/content/admin/remove?cache=pages&id=" },
                                  { "removeTemplateCache", "/content/admin/remove?cache=templates&id=" },
                                  { "fileTransferFolder", "/usr/local/tomcat/webapps/content/fileTransferFolder" },
                                  { "lookInContext", 1 },
                                  { "adminGroupID", 4 },
                                  { "betaServer", true } } } } } },
                        { "servlet-mapping",
                          { { "cofaxCDS", "/" },
                            { "cofaxEmail", "/cofaxutil/aemail/*" },
                            { "cofaxAdmin", "/admin/*" },
                            { "fileServlet", "/static/*" },
                            { "cofaxTools", "/tools/*" } } },

                        { "taglib",
                          { { "taglib-uri", "cofax.tld" }, { "taglib-location", "/WEB-INF/tlds/cofax.tld" } } } } }
                };
                CHECK(j == expect);
            },
            [](json& j)
            {
                // json expect = {};
                // CHECK(j == expect);
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
                    return std::char_traits<char>::eof();
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
