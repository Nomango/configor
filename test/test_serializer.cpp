// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <fstream>
#include <iomanip>

using namespace jsonxx;

class SerializerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        j = {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Nomango"},
            {"nothing", nullptr},
            {"answer", {
                {"everything", 42}
            }},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"value", 42.99}
            }}
        };
    }

    json j;
};

TEST_F(SerializerTest, test_write_to_file)
{
    // write prettified JSON to another file
    std::ofstream o("output/pretty.json");
    o << std::setw(4) << j << std::endl;
    o.close();
}

TEST_F(SerializerTest, test_dump)
{
    std::string serialized_string = j.dump();
    (void)serialized_string;
}

TEST_F(SerializerTest, test_pretty_dump)
{
    std::string serialized_string = j.dump(4, ' ');
    (void)serialized_string;
}
