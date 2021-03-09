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
            }},
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
    ASSERT_NO_THROW(std::string serialized_string = j.dump());
    ASSERT_NO_THROW(std::string serialized_string = j.dump(4, ' '));

    // issue 7
    ASSERT_EQ(json(0.0).dump(), "0.0");
    ASSERT_EQ(json(1.0).dump(), "1.0");
    ASSERT_EQ(json(1.2).dump(), "1.2");
    ASSERT_EQ(json(1.23).dump(), "1.23");
}
