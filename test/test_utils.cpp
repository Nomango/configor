// Copyright (c) 2019 Nomango

#include <gtest/gtest.h>
#include <jsonxx/json.hpp>

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
    Driver driver_;
    std::vector<Passenger> passengers_;

public:
    Bus() = default;
    Bus(int license, const Driver& driver, const std::vector<Passenger>& passengers) : license_(license), driver_(driver), passengers_(passengers) {}

    bool operator==(const Bus& rhs) const
    {
        return license_ == rhs.license_ && driver_ == rhs.driver_ && passengers_ == rhs.passengers_;
    }
};

template<>
class json_bind<Driver>
{
public:
    void to_json(json& j, const Driver& v)
    {
        j["name"] = v.name_;
    }

    void from_json(Driver& v, const json& j)
    {
        v.name_ = j["name"].as_string();
    }
};

template<>
class json_bind<Passenger>
{
public:
    void to_json(json& j, const Passenger& v)
    {
        j["name"] = v.name_;
        j["age"] = v.age_;
    }

    void from_json(Passenger& v, const json& j)
    {
        v.name_ = j["name"].as_string();
        v.age_ = (int)j["age"];
    }
};

template<>
class json_bind<std::vector<Passenger>>
{
public:
    void to_json(json& j, const std::vector<Passenger>& v)
    {
        j = json_type::array;
        for (size_t i = 0; i < v.size(); i++)
        {
            jsonxx::to_json(j[i], v[i]);
        }
    }

    void from_json(std::vector<Passenger>& v, const json& j)
    {
        v.resize(j.size());
        for (size_t i = 0; i < j.size(); i++)
        {
            jsonxx::from_json(j[i], v[i]);
        }
    }
};

template<>
class json_bind<Bus>
{
public:
    void to_json(json& j, const Bus& v)
    {
        j["license"] = v.license_;
        jsonxx::to_json(j["driver"], v.driver_);
        jsonxx::to_json(j["passengers"], v.passengers_);
    }

    void from_json(Bus& v, const json& j)
    {
        v.license_ = (int)j["license"];
        jsonxx::from_json(j["driver"], v.driver_);
        jsonxx::from_json(j["passengers"], v.passengers_);
    }
};

TEST(test_to_json, test_to_json)
{
    Bus bus = Bus{
        100,
        Driver{ "driver" },
        { Passenger{ "p1", 18 }, Passenger{ "p2", 54 } },
    };

    json j;
    to_json(j, bus);

    ASSERT_TRUE(j.is_object());
    ASSERT_EQ(j.size(), 3);
    ASSERT_EQ(j["license"], 100);
    ASSERT_EQ(j["driver"]["name"], "driver");
    ASSERT_TRUE(j["passengers"].is_array());
    ASSERT_EQ(j["passengers"].size(), 2);
    ASSERT_EQ(j["passengers"][0]["name"], "p1");
    ASSERT_EQ(j["passengers"][0]["age"], 18);
    ASSERT_EQ(j["passengers"][1]["name"], "p2");
    ASSERT_EQ(j["passengers"][1]["age"], 54);
}

TEST(test_from_json, test_from_json)
{
    json j = {
        {"license", 100},
        {"driver", {{"name", "driver"}} },
        {"passengers", {
            {{"name", "p1"}, {"age", 18}},
            {{"name", "p2"}, {"age", 54}},
        }},
    };

    Bus bus;
    from_json(j, bus);

    Bus expect = Bus{
        100,
        Driver{ "driver" },
        { Passenger{ "p1", 18 }, Passenger{ "p2", 54 } },
    };

    ASSERT_TRUE(bus == expect);
}
