// Copyright (c) 2021 Nomango

#include "common.h"

TEST_CASE("test_iterator")
{
    SECTION("string")
    {
        value c = "string";

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

        CHECK((std::size_t)std::distance(c.begin(), c.end()) == c.size());
        CHECK((std::size_t)std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("integer")
    {
        value c = 100;

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

        CHECK((std::size_t)std::distance(c.begin(), c.end()) == c.size());
        CHECK((std::size_t)std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("double")
    {
        value c = 100.0;

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

        CHECK((std::size_t)std::distance(c.begin(), c.end()) == c.size());
        CHECK((std::size_t)std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("boolean")
    {
        value c = true;

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

        CHECK((std::size_t)std::distance(c.begin(), c.end()) == c.size());
        CHECK((std::size_t)std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("null")
    {
        value c = nullptr;

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

        CHECK((std::size_t)std::distance(c.begin(), c.end()) == c.size());
        CHECK((std::size_t)std::distance(c.rbegin(), c.rend()) == c.size());

        CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        CHECK_THROWS_AS(c.erase(c.begin()), configor_invalid_iterator);
    }

    SECTION("object")
    {
        const value obj = make_object<value>({ { "user", make_object<value>({ { "id", 1 }, { "name", "Nomango" } }) } });
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

        CHECK((std::size_t)std::distance(obj.begin(), obj.end()) == obj.size());
        CHECK((std::size_t)std::distance(obj.rbegin(), obj.rend()) == obj.size());

        {
            value c = make_object<value>({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase("1"));
            CHECK(c == make_object<value>({ { "2", 2 } }));
        }
        {
            value c = make_object<value>({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(c.begin()));
            CHECK(c == make_object<value>({ { "2", 2 } }));
        }
        {
            value c = make_object<value>({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(c.begin(), c.end()));
            CHECK(c == make_object<value>({}));
        }
        {
            value c = make_object<value>({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(c.find("1")));
            CHECK(c == make_object<value>({ { "2", 2 } }));
        }
        {
            value c = make_object<value>({ { "1", 1 }, { "2", 2 } });
            CHECK_NOTHROW(c.erase(const_cast<const value&>(c).find("1")));
            CHECK(c == make_object<value>({ { "2", 2 } }));
        }
        {
            value c = make_object<value>({});
            CHECK_THROWS_AS(c.erase(0), configor_invalid_key);
        }
    }

    SECTION("array")
    {
        const value arr = make_array<value>({ 1, 2, 3 });

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

        CHECK((std::size_t)std::distance(arr.begin(), arr.end()) == arr.size());
        CHECK((std::size_t)std::distance(arr.rbegin(), arr.rend()) == arr.size());

        {
            value c = make_array<value>({ 1, 2, 3 });
            CHECK_NOTHROW(c.erase(0));
            CHECK(c == make_array<value>({ 2, 3 }));
        }
        {
            value c = make_array<value>({ 1, 2, 3 });
            CHECK_NOTHROW(c.erase(c.begin()));
            CHECK(c == make_array<value>({ 2, 3 }));
        }
        {
            value c = make_array<value>({ 1, 2, 3 });
            CHECK_NOTHROW(c.erase(c.begin(), c.end()));
            CHECK(c == make_array<value>({}));
        }
        {
            value c = make_array<value>({});
            CHECK_THROWS_AS(c.erase(""), configor_invalid_key);
        }
    }

    SECTION("others")
    {
        CHECK(make_object<value>({}).begin() != make_array<value>({}).begin());
        CHECK_THROWS_AS((value::iterator{ nullptr }).key(), configor_invalid_iterator);
        CHECK_FALSE(value::iterator{ nullptr } == value::iterator{ nullptr });
    }
}
