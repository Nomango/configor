// Copyright (c) 2021 Nomango

#include "common.h"

TEST_CASE("test_iterator")
{
    SECTION("string")
    {
        config c = "string";

        auto iter = c.begin();
        CHECK(iter != c.end());
        CHECK(iter == c.begin());
        CHECK(*iter == c);

        CHECK_THROWS_AS(iter.key(), configor_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == c.end());
        CHECK(iter != c.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != c.end());
        CHECK(iter == c.begin());

        CHECK_THROWS_AS(c.end().operator*(), std::out_of_range);

        CHECK(std::distance(c.begin(), c.end()) == c.size());
        CHECK(std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("integer")
    {
        config c = 100;

        auto iter = c.begin();
        CHECK(iter != c.end());
        CHECK(iter == c.begin());
        CHECK(*iter == c);

        CHECK_THROWS_AS(iter.key(), configor_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == c.end());
        CHECK(iter != c.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != c.end());
        CHECK(iter == c.begin());

        CHECK_THROWS_AS(c.end().operator*(), std::out_of_range);

        CHECK(std::distance(c.begin(), c.end()) == c.size());
        CHECK(std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("double")
    {
        config c = 100.0;

        auto iter = c.begin();
        CHECK(iter != c.end());
        CHECK(iter == c.begin());
        CHECK(*iter == c);

        CHECK_THROWS_AS(iter.key(), configor_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == c.end());
        CHECK(iter != c.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != c.end());
        CHECK(iter == c.begin());

        CHECK_THROWS_AS(c.end().operator*(), std::out_of_range);

        CHECK(std::distance(c.begin(), c.end()) == c.size());
        CHECK(std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("boolean")
    {
        config c = true;

        auto iter = c.begin();
        CHECK(iter != c.end());
        CHECK(iter == c.begin());
        CHECK(*iter == c);

        CHECK_THROWS_AS(iter.key(), configor_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == c.end());
        CHECK(iter != c.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != c.end());
        CHECK(iter == c.begin());

        CHECK_THROWS_AS(c.end().operator*(), std::out_of_range);

        CHECK(std::distance(c.begin(), c.end()) == c.size());
        CHECK(std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("null")
    {
        config c = nullptr;

        auto iter = c.begin();
        CHECK(iter == c.end());
        CHECK(iter == c.begin());
        CHECK_THROWS_AS(*iter, std::out_of_range);

        CHECK_THROWS_AS(iter.operator*(), std::out_of_range);

        CHECK_NOTHROW(iter++);
        CHECK(iter == c.end());
        CHECK(iter == c.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter == c.end());
        CHECK(iter == c.begin());

        CHECK(std::distance(c.begin(), c.end()) == c.size());
        CHECK(std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("object")
    {
        const config obj = config::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
        for (auto iter = obj.begin(); iter != obj.end(); iter++)
        {
            CHECK(iter == obj.find(iter.key()));
            CHECK(iter.value() == obj[iter.key()]);
            CHECK(iter.value() == *iter);
        }

        for (auto riter = obj.rbegin(); riter != obj.rend(); riter++)
        {
            CHECK(riter.value() == obj[riter.key()]);
        }

        CHECK_THROWS_AS(obj.end().operator*(), std::out_of_range);

        CHECK(obj.find("user") != obj.end());
        CHECK(obj.find("missing") == obj.end());

        CHECK(std::distance(obj.begin(), obj.end()) == obj.size());
        CHECK(std::distance(obj.rbegin(), obj.rend()) == obj.size());

        {
            config c = config::object({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase("1"));
            CHECK(c == config::object({ { "2", 2 } }));
        }
        {
            config c = config::object({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(c.begin()));
            CHECK(c == config::object({ { "2", 2 } }));
        }
        {
            config c = config::object({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(c.begin(), c.end()));
            CHECK(c == config::object({}));
        }
        {
            config c = config::object({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(c.find("1")));
            CHECK(c == config::object({ { "2", 2 } }));
        }
        {
            config c = config::object({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(const_cast<const config&>(c).find("1")));
            CHECK(c == config::object({ { "2", 2 } }));
        }
        {
            config c = config::object({});
            CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        }
    }

    SECTION("array")
    {
        const config arr = config::array({ 1, 2, 3 });

        size_t idx = 0;
        for (auto iter = arr.begin(); iter != arr.end(); iter++, idx++)
        {
            auto temp_iter = arr.begin();
            std::advance(temp_iter, idx);
            CHECK(iter == temp_iter);
            CHECK(iter.value() == arr[idx]);
            CHECK_THROWS_AS(iter.key(), configor_invalid_iterator);
        }

        idx = 0;
        for (auto elem : arr)
        {
            CHECK(elem == arr[idx]);
            idx++;
        }

        idx = arr.size() - 1;
        for (auto riter = arr.rbegin(); riter != arr.rend(); riter++, idx--)
        {
            CHECK(riter.value() == arr[idx]);
        }

        CHECK_THROWS_AS(arr.end().operator*(), std::out_of_range);

        CHECK(std::distance(arr.begin(), arr.end()) == arr.size());
        CHECK(std::distance(arr.rbegin(), arr.rend()) == arr.size());

        {
            config c = config::array({ 1, 2, 3 });
            CHECK_NOTHROW(c.erase(0));
            CHECK(c == config::array({ 2, 3 }));
        }
        {
            config c = config::array({ 1, 2, 3 });
            CHECK_NOTHROW(c.erase(c.begin()));
            CHECK(c == config::array({ 2, 3 }));
        }
        {
            config c = config::array({ 1, 2, 3 });
            CHECK_NOTHROW(c.erase(c.begin(), c.end()));
            CHECK(c == config::array({}));
        }
        {
            config c = config::array({});
            CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        }
    }

    SECTION("others")
    {
        CHECK(config::object({}).begin() != config::array({}).begin());
        CHECK_THROWS_AS((config::iterator{ nullptr }).key(), configor_invalid_iterator);
        CHECK_FALSE(config::iterator{ nullptr } == config::iterator{ nullptr });
    }
}
