// Copyright (c) 2019 Nomango

#include "common.h"

#include <sstream>  // std::stringstream

class Driver
{
    friend json_bind<Driver>;

    std::string name_;

public:
    Driver(const std::string& name)
        : name_(name)
    {
    }

    bool operator==(const Driver& rhs) const
    {
        return name_ == rhs.name_;
    }
};

template <>
struct jsonxx::json_bind<Driver>
{
    static void to_json(json& j, const Driver& v)
    {
        j["name"] = v.name_;
    }

    static Driver from_json(const json& j)
    {
        return Driver(j["name"].get<std::string>());
    }
};

class Passenger
{
    friend json_bind<Passenger>;

    std::string name_;
    int         age_ = 0;

public:
    Passenger() = default;
    Passenger(const std::string& name, int age)
        : name_(name)
        , age_(age)
    {
    }

    bool operator==(const Passenger& rhs) const
    {
        return name_ == rhs.name_ && age_ == rhs.age_;
    }
};

template <>
struct jsonxx::json_bind<Passenger>
{
    static void to_json(json& j, const Passenger& v)
    {
        j["name"] = v.name_;
        j["age"]  = v.age_;
    }

    static void from_json(const json& j, Passenger& v)
    {
        v.name_ = j["name"].get<std::string>();
        v.age_  = (int)j["age"];
    }
};

using PassengerPtr = std::shared_ptr<Passenger>;
using DriverPtr    = std::unique_ptr<Driver>;

class Bus
{
    friend json_bind<Bus>;

    int                                 license_ = 0;
    DriverPtr                           driver_;
    std::vector<PassengerPtr>           passengers_;
    std::map<std::string, PassengerPtr> olders_;

public:
    Bus() = default;
    Bus(int license, DriverPtr&& driver, const std::vector<PassengerPtr>& passengers,
        const std::map<std::string, PassengerPtr>& olders)
        : license_(license)
        , driver_(std::move(driver))
        , passengers_(passengers)
        , olders_(olders)
    {
    }

    Bus(const Bus& rhs)
        : license_(rhs.license_)
        , driver_(rhs.driver_ ? new Driver(*rhs.driver_) : nullptr)
        , passengers_(rhs.passengers_)
        , olders_(rhs.olders_)
    {
    }

    bool operator==(const Bus& rhs) const
    {
        using pair = std::pair<const std::string, PassengerPtr const>;
        return license_ == rhs.license_ && *driver_ == *rhs.driver_ && passengers_.size() == rhs.passengers_.size()
               && std::equal(passengers_.cbegin(), passengers_.cend(), rhs.passengers_.cbegin(),
                             [](const PassengerPtr& p1, const PassengerPtr& p2)
                             { return p1 ? (*p1 == *p2) : (p2 == nullptr); })
               && olders_.size() == rhs.olders_.size()
               && std::equal(olders_.cbegin(), olders_.cend(), rhs.olders_.cbegin(),
                             [](const pair& p1, const pair& p2)
                             { return p1.first == p2.first && *p1.second == *p2.second; });
    }

    Bus& operator=(const Bus& rhs)
    {
        license_ = rhs.license_;
        driver_.reset(rhs.driver_ ? new Driver(*rhs.driver_) : nullptr);
        passengers_ = rhs.passengers_;
        olders_     = rhs.olders_;
        return *this;
    }

public:
    JSONXX_BIND(Bus, license_, driver_, passengers_, olders_)
};

class ConversionTest : public testing::Test
{
protected:
    Bus  expect_bus;
    json expect_json;

    void SetUp() override
    {
        expect_bus = Bus{
            100,
            std::unique_ptr<Driver>(new Driver("driver")),
            { std::make_shared<Passenger>("p1", 18), std::make_shared<Passenger>("p2", 54), nullptr },
            { { "p2", std::make_shared<Passenger>("p2", 54) } },
        };

        expect_json = {
            { "license_", 100 },
            { "driver_", { { "name", "driver" } } },
            { "passengers_",
              {
                  { { "name", "p1" }, { "age", 18 } },
                  { { "name", "p2" }, { "age", 54 } },
                  nullptr,
              } },
            { "olders_",
              {
                  { "p2", { { "name", "p2" }, { "age", 54 } } },
              } },
        };
    }
};

TEST_F(ConversionTest, test_to_json)
{
    json j = expect_bus;

    ASSERT_TRUE(j == expect_json);
}

TEST_F(ConversionTest, test_from_json)
{
    Bus bus = Bus(expect_json);

    ASSERT_TRUE(bus == expect_bus);
}

TEST_F(ConversionTest, test_json_wrap)
{
    std::stringstream s;
    s << json_wrap(expect_bus);
    ASSERT_EQ(s.str(), expect_json.dump());

    Bus bus;
    s >> json_wrap(bus);
    ASSERT_TRUE(bus == expect_bus);
}

TEST_F(ConversionTest, test_containers)
{
    {
        std::array<Bus, 1> v;
        v[0] = expect_bus;

        json j;
        ASSERT_NO_THROW(j = v);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 1);
        ASSERT_EQ(j[0], expect_json);

        v[0] = Bus{};
        ASSERT_NO_THROW(v = j);
        ASSERT_EQ(v[0], expect_bus);
    }

    {
        std::vector<Bus> v;
        v.push_back(expect_bus);

        json j;
        ASSERT_NO_THROW(j = v);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 1);
        ASSERT_EQ(j[0], expect_json);

        v.clear();
        ASSERT_NO_THROW(v = j);
        ASSERT_EQ(v.size(), 1);
        ASSERT_EQ(v[0], expect_bus);
    }

    {
        std::deque<Bus> v;
        v.push_back(expect_bus);

        json j;
        ASSERT_NO_THROW(j = v);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 1);
        ASSERT_EQ(j[0], expect_json);

        v.clear();
        ASSERT_NO_THROW(v = j);
        ASSERT_EQ(v.size(), 1);
        ASSERT_EQ(v[0], expect_bus);
    }

    {
        std::list<Bus> v;
        v.push_back(expect_bus);

        json j;
        ASSERT_NO_THROW(j = v);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 1);
        ASSERT_EQ(j[0], expect_json);

        v.clear();
        ASSERT_NO_THROW(v = j);
        ASSERT_EQ(v.size(), 1);
        ASSERT_EQ(v.front(), expect_bus);
    }

    {
        std::forward_list<Bus> v;
        v.push_front(expect_bus);

        json j;
        ASSERT_NO_THROW(j = v);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 1);
        ASSERT_EQ(j[0], expect_json);

        v.clear();
        ASSERT_NO_THROW(v = j);
        ASSERT_EQ(v.front(), expect_bus);
    }

    {
        std::set<int> expect = { 1, 1, 2, 3 };

        json j;
        ASSERT_NO_THROW(j = expect);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 3);
        ASSERT_EQ(j, expect);

        std::set<int> actual;
        ASSERT_NO_THROW(actual = j);
        ASSERT_EQ(actual, expect);
    }

    {
        std::unordered_set<int> expect = { 1, 1, 2, 3 };

        json j;
        ASSERT_NO_THROW(j = expect);
        ASSERT_TRUE(j.is_array());
        ASSERT_EQ(j.size(), 3);
        ASSERT_EQ(j, expect);

        std::unordered_set<int> actual;
        ASSERT_NO_THROW(actual = j);
        ASSERT_EQ(actual, expect);
    }

    {
        std::map<std::string, int> expect = { { "one", 1 }, { "two", 2 } };

        json j;
        ASSERT_NO_THROW(j = expect);
        ASSERT_TRUE(j.is_object());
        ASSERT_EQ(j.size(), 2);
        ASSERT_EQ(j["one"], 1);
        ASSERT_EQ(j["two"], 2);

        std::map<std::string, int> actual;
        ASSERT_NO_THROW(actual = j);
        ASSERT_EQ(actual, expect);
    }

    {
        std::unordered_map<std::string, int> expect = { { "one", 1 }, { "two", 2 } };

        json j;
        ASSERT_NO_THROW(j = expect);
        ASSERT_TRUE(j.is_object());
        ASSERT_EQ(j.size(), 2);
        ASSERT_EQ(j["one"], 1);
        ASSERT_EQ(j["two"], 2);

        std::unordered_map<std::string, int> actual;
        ASSERT_NO_THROW(actual = j);
        ASSERT_EQ(actual, expect);
    }
}

TEST(test_conversion, test_nullptr)
{
    {
        json j;

        std::unique_ptr<Passenger> pnull = nullptr;
        ASSERT_NO_THROW(j = pnull);
        ASSERT_TRUE(j.is_null());

        auto pnotnull = std::unique_ptr<Passenger>(new Passenger);
        ASSERT_NO_THROW(pnotnull = j);
        ASSERT_TRUE(pnotnull == nullptr);
    }

    {
        json j;

        std::shared_ptr<Passenger> pnull = nullptr;
        ASSERT_NO_THROW(j = pnull);
        ASSERT_TRUE(j.is_null());

        auto pnotnull = std::shared_ptr<Passenger>(new Passenger);
        ASSERT_NO_THROW(pnotnull = j);
        ASSERT_TRUE(pnotnull == nullptr);
    }
}
