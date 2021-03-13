// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>
#include <sstream>

using namespace jsonxx;

class Driver
{
    friend json_bind<Driver>;

    std::string name_;

public:
    Driver() = default;
    Driver(const std::string& name) : name_(name) {}

    bool operator==(const Driver& rhs) const
    {
        return name_ == rhs.name_;
    }
};

class Passenger
{
    friend json_bind<Passenger>;

    std::string name_;
    int age_ = 0;

public:
    Passenger() = default;
    Passenger(const std::string& name, int age) : name_(name), age_(age) {}

    bool operator==(const Passenger& rhs) const
    {
        return name_ == rhs.name_ && age_ == rhs.age_;
    }
};

class Bus
{
    friend json_bind<Bus>;

    int license_ = 0;
    Driver* driver_ = nullptr;
    std::vector<Passenger*> passengers_;
    std::map<std::string, Passenger*> olders_;

public:
    Bus() = default;
    Bus(int license, Driver* driver, const std::vector<Passenger*>& passengers, const std::map<std::string, Passenger*>& olders)
        : license_(license), driver_(driver), passengers_(passengers), olders_(olders)
    {
    }

    bool operator==(const Bus& rhs) const
    {
        using pair = std::pair<const std::string, Passenger* const>;
        return license_ == rhs.license_
            && *driver_ == *rhs.driver_
            && passengers_.size() == rhs.passengers_.size()
            && std::equal(passengers_.cbegin(), passengers_.cend(), rhs.passengers_.cbegin(), [](const Passenger* p1, const Passenger* p2) { return p1 ? (*p1 == *p2) : (p2 == nullptr); })
            && olders_.size() == rhs.olders_.size()
            && std::equal(olders_.cbegin(), olders_.cend(), rhs.olders_.cbegin(), [](const pair& p1, const pair& p2) { return p1.first == p2.first && *p1.second == *p2.second; })
            ;
    }
};

template<>
struct json_bind<Driver>
{
    void to_json(json& j, const Driver& v)
    {
        j["name"] = v.name_;
    }

    void from_json(const json& j, Driver& v)
    {
        v.name_ = j["name"].as_string();
    }
};

template<>
struct json_bind<Passenger>
{
    void to_json(json& j, const Passenger& v)
    {
        j["name"] = v.name_;
        j["age"] = v.age_;
    }

    void from_json(const json& j, Passenger& v)
    {
        v.name_ = j["name"].as_string();
        v.age_ = (int)j["age"];
    }
};

template<>
struct json_bind<Bus>
{
    void to_json(json& j, const Bus& v)
    {
        j["license"] = v.license_;
        jsonxx::to_json(j["driver"], v.driver_);
        jsonxx::to_json(j["passengers"], v.passengers_);
        jsonxx::to_json(j["olders"], v.olders_);
    }

    void from_json(const json& j, Bus& v)
    {
        v.license_ = (int)j["license"];
        jsonxx::from_json(j["driver"], v.driver_);
        jsonxx::from_json(j["passengers"], v.passengers_);
        jsonxx::from_json(j["olders"], v.olders_);
    }
};

class UtilsTest : public testing::Test
{
protected:
    Bus expect_bus;
    json expect_json;

    void SetUp() override
    {
        expect_bus = Bus{
            100,
            new Driver{ "driver" },
            { new Passenger{ "p1", 18 }, new Passenger{ "p2", 54 }, nullptr },
            { {"p2", new Passenger{ "p2", 54 }} },
        };

        expect_json = {
            {"license", 100},
            {"driver", {{"name", "driver"}} },
            {"passengers", {
                {{"name", "p1"}, {"age", 18}},
                {{"name", "p2"}, {"age", 54}},
                nullptr,
            }},
            {"olders", {
                {"p2", {{"name", "p2"}, {"age", 54}} },
            }},
        };
    }
};

TEST_F(UtilsTest, test_to_json)
{
    json j;
    to_json(j, expect_bus);

    ASSERT_TRUE(j.is_object());
    ASSERT_EQ(j.size(), 4);
    ASSERT_EQ(j["license"], 100);
    ASSERT_EQ(j["driver"]["name"], "driver");
    ASSERT_TRUE(j["passengers"].is_array());
    ASSERT_EQ(j["passengers"].size(), 3);
    ASSERT_EQ(j["passengers"][0]["name"], "p1");
    ASSERT_EQ(j["passengers"][0]["age"], 18);
    ASSERT_EQ(j["passengers"][1]["name"], "p2");
    ASSERT_EQ(j["passengers"][1]["age"], 54);
    ASSERT_TRUE(j["passengers"][2].is_null());
    ASSERT_EQ(j["olders"].size(), 1);
    ASSERT_EQ(j["olders"]["p2"]["name"], "p2");
    ASSERT_EQ(j["olders"]["p2"]["age"], 54);
}

TEST_F(UtilsTest, test_from_json)
{
    Bus bus;
    from_json(expect_json, bus);

    ASSERT_TRUE(bus == expect_bus);
}

TEST_F(UtilsTest, test_json_wrap)
{
    std::stringstream s;
    s << json_wrap(expect_bus);
    ASSERT_EQ(s.str(), expect_json.dump());

    Bus bus;
    s >> json_wrap(bus);
    ASSERT_TRUE(bus == expect_bus);
}
