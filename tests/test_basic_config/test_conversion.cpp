// Copyright (c) 2019 Nomango

#include "common.h"

#include <sstream>  // std::stringstream

class Driver
{
    friend config_binder<Driver>;

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

namespace configor
{
template <>
struct config_binder<Driver>
{
    static void to_config(config& c, const Driver& v)
    {
        c["name"] = v.name_;
    }

    static Driver from_config(const config& c)
    {
        return Driver(c["name"].get<std::string>());
    }
};
}  // namespace configor

class Passenger
{
    friend config_binder<Passenger>;

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

namespace configor
{
template <>
struct config_binder<Passenger>
{
    static void to_config(config& c, const Passenger& v)
    {
        c["name"] = v.name_;
        c["age"]  = v.age_;
    }

    static void from_config(const config& c, Passenger& v)
    {
        c["name"].get(v.name_);
        c["age"].get(v.age_);
    }
};
}  // namespace configor

using PassengerPtr = std::shared_ptr<Passenger>;
using DriverPtr    = std::unique_ptr<Driver>;

class Bus
{
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

    CONFIGOR_BIND(config, Bus, REQUIRED(license_, "license"), REQUIRED(driver_, "driver"),
                  OPTIONAL(passengers_, "passengers"), OPTIONAL(olders_, "olders"))

private:
    int                                 license_ = 0;
    DriverPtr                           driver_;
    std::vector<PassengerPtr>           passengers_;
    std::map<std::string, PassengerPtr> olders_;
};

class ConversionTest
{
protected:
    Bus    expect_bus;
    config expect_config;

    ConversionTest()
    {
        expect_bus = Bus{
            100,
            std::unique_ptr<Driver>(new Driver("driver")),
            { std::make_shared<Passenger>("p1", 18), std::make_shared<Passenger>("p2", 54), nullptr },
            { { "p2", std::make_shared<Passenger>("p2", 54) } },
        };

        expect_config = {
            { "license", 100 },
            { "driver", { { "name", "driver" } } },
            { "passengers",
              {
                  { { "name", "p1" }, { "age", 18 } },
                  { { "name", "p2" }, { "age", 54 } },
                  nullptr,
              } },
            { "olders",
              {
                  { "p2", { { "name", "p2" }, { "age", 54 } } },
              } },
        };
    }
};

TEST_CASE_METHOD(ConversionTest, "test_to_config")
{
    config c = expect_bus;

    CHECK(c == expect_config);
}

TEST_CASE_METHOD(ConversionTest, "test_from_config")
{
    Bus bus = Bus(expect_config);

    CHECK(bus == expect_bus);
}

TEST_CASE_METHOD(ConversionTest, "test_containers")
{
    {
        std::array<Bus, 1> v;
        v[0] = expect_bus;

        config c;
        CHECK_NOTHROW(c = v);
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0] == expect_config);

        v[0] = Bus{};
        CHECK_NOTHROW(v = c);
        CHECK(v[0] == expect_bus);
    }

    {
        std::vector<Bus> v;
        v.push_back(expect_bus);

        config c;
        CHECK_NOTHROW(c = v);
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0] == expect_config);

        v.clear();
        CHECK_NOTHROW(v = c);
        CHECK(v.size() == 1);
        CHECK(v[0] == expect_bus);
    }

    {
        std::deque<Bus> v;
        v.push_back(expect_bus);

        config c;
        CHECK_NOTHROW(c = v);
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0] == expect_config);

        v.clear();
        CHECK_NOTHROW(v = c);
        CHECK(v.size() == 1);
        CHECK(v[0] == expect_bus);
    }

    {
        std::list<Bus> v;
        v.push_back(expect_bus);

        config c;
        CHECK_NOTHROW(c = v);
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0] == expect_config);

        v.clear();
        CHECK_NOTHROW(v = c);
        CHECK(v.size() == 1);
        CHECK(v.front() == expect_bus);
    }

    {
        std::forward_list<Bus> v;
        v.push_front(expect_bus);

        config c;
        CHECK_NOTHROW(c = v);
        CHECK(c.is_array());
        CHECK(c.size() == 1);
        CHECK(c[0] == expect_config);

        v.clear();
        CHECK_NOTHROW(v = c);
        CHECK(v.front() == expect_bus);
    }

    {
        std::set<int> expect = { 1, 1, 2, 3 };

        config c;
        CHECK_NOTHROW(c = expect);
        CHECK(c.is_array());
        CHECK(c.size() == 3);
        CHECK(c == expect);

        std::set<int> actual;
        CHECK_NOTHROW(actual = c);
        CHECK(actual == expect);
    }

    {
        std::unordered_set<int> expect = { 1, 1, 2, 3 };

        config c;
        CHECK_NOTHROW(c = expect);
        CHECK(c.is_array());
        CHECK(c.size() == 3);
        CHECK(c == expect);

        std::unordered_set<int> actual;
        CHECK_NOTHROW(actual = c);
        CHECK(actual == expect);
    }

    {
        std::map<std::string, int> expect = { { "one", 1 }, { "two", 2 } };

        config c;
        CHECK_NOTHROW(c = expect);
        CHECK(c.is_object());
        CHECK(c.size() == 2);
        CHECK(c["one"] == 1);
        CHECK(c["two"] == 2);

        std::map<std::string, int> actual;
        CHECK_NOTHROW(actual = c);
        CHECK(actual == expect);
    }

    {
        std::unordered_map<std::string, int> expect = { { "one", 1 }, { "two", 2 } };

        config c;
        CHECK_NOTHROW(c = expect);
        CHECK(c.is_object());
        CHECK(c.size() == 2);
        CHECK(c["one"] == 1);
        CHECK(c["two"] == 2);

        std::unordered_map<std::string, int> actual;
        CHECK_NOTHROW(actual = c);
        CHECK(actual == expect);
    }
}

struct CStylePassengers
{
    Passenger passengers[2];

    CONFIGOR_BIND_ALL_REQUIRED(config, CStylePassengers, passengers);
};

TEST_CASE("test_conversion")
{
    SECTION("test unique_ptr")
    {
        config c;

        std::unique_ptr<Passenger> pnull = nullptr;
        CHECK_NOTHROW(c = pnull);
        CHECK(c.is_null());

        auto pnotnull = std::unique_ptr<Passenger>(new Passenger);
        CHECK_NOTHROW(pnotnull = c);
        CHECK(pnotnull == nullptr);
    }

    SECTION("test shared_ptr")
    {
        config c;

        std::shared_ptr<Passenger> pnull = nullptr;
        CHECK_NOTHROW(c = pnull);
        CHECK(c.is_null());

        auto pnotnull = std::shared_ptr<Passenger>(new Passenger);
        CHECK_NOTHROW(pnotnull = c);
        CHECK(pnotnull == nullptr);
    }

    SECTION("test c-style array")
    {
        config c;

        CStylePassengers v = { Passenger{ "1", 1 }, Passenger{ "2", 2 } };
        CHECK_NOTHROW(c = v);
        CHECK(c.is_object());
        CHECK(c["passengers"].is_array());
        CHECK(c["passengers"].size() == 2);

        CStylePassengers v2;
        CHECK_NOTHROW(v2 = c);
        CHECK(v2.passengers[0] == v.passengers[0]);
        CHECK(v2.passengers[1] == v.passengers[1]);
    }
}
