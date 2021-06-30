// Copyright (c) 2021 Nomango

#include "common.h"

TEST_CASE("test_iterator")
{
    SECTION("string")
    {
        json j = "string";

        auto iter = j.begin();
        CHECK(iter != j.end());
        CHECK(iter == j.begin());
        CHECK(*iter == j);

        CHECK_THROWS_AS(iter.key(), json_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == j.end());
        CHECK(iter != j.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != j.end());
        CHECK(iter == j.begin());

        CHECK_THROWS_AS(j.end().operator*(), std::out_of_range);

        CHECK(std::distance(j.begin(), j.end()) == j.size());
        CHECK(std::distance(j.rbegin(), j.rend()) == j.size());
    }

    SECTION("integer")
    {
        json j = 100;

        auto iter = j.begin();
        CHECK(iter != j.end());
        CHECK(iter == j.begin());
        CHECK(*iter == j);

        CHECK_THROWS_AS(iter.key(), json_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == j.end());
        CHECK(iter != j.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != j.end());
        CHECK(iter == j.begin());

        CHECK_THROWS_AS(j.end().operator*(), std::out_of_range);

        CHECK(std::distance(j.begin(), j.end()) == j.size());
        CHECK(std::distance(j.rbegin(), j.rend()) == j.size());
    }

    SECTION("double")
    {
        json j = 100.0;

        auto iter = j.begin();
        CHECK(iter != j.end());
        CHECK(iter == j.begin());
        CHECK(*iter == j);

        CHECK_THROWS_AS(iter.key(), json_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == j.end());
        CHECK(iter != j.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != j.end());
        CHECK(iter == j.begin());

        CHECK_THROWS_AS(j.end().operator*(), std::out_of_range);

        CHECK(std::distance(j.begin(), j.end()) == j.size());
        CHECK(std::distance(j.rbegin(), j.rend()) == j.size());
    }

    SECTION("boolean")
    {
        json j = true;

        auto iter = j.begin();
        CHECK(iter != j.end());
        CHECK(iter == j.begin());
        CHECK(*iter == j);

        CHECK_THROWS_AS(iter.key(), json_invalid_iterator);

        CHECK_NOTHROW(iter++);
        CHECK(iter == j.end());
        CHECK(iter != j.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter != j.end());
        CHECK(iter == j.begin());

        CHECK_THROWS_AS(j.end().operator*(), std::out_of_range);

        CHECK(std::distance(j.begin(), j.end()) == j.size());
        CHECK(std::distance(j.rbegin(), j.rend()) == j.size());
    }

    SECTION("null")
    {
        json j = nullptr;

        auto iter = j.begin();
        CHECK(iter == j.end());
        CHECK(iter == j.begin());
        CHECK_THROWS_AS(*iter, std::out_of_range);

        CHECK_THROWS_AS(iter.operator*(), std::out_of_range);

        CHECK_NOTHROW(iter++);
        CHECK(iter == j.end());
        CHECK(iter == j.begin());
        CHECK_NOTHROW(iter--);
        CHECK(iter == j.end());
        CHECK(iter == j.begin());

        CHECK(std::distance(j.begin(), j.end()) == j.size());
        CHECK(std::distance(j.rbegin(), j.rend()) == j.size());
    }

    SECTION("object")
    {
        const json obj = json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
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
    }

    SECTION("array")
    {
        const json arr = json::array({ 1, 2, 3 });

        size_t idx = 0;
        for (auto iter = arr.begin(); iter != arr.end(); iter++, idx++)
        {
            auto temp_iter = arr.begin();
            std::advance(temp_iter, idx);
            CHECK(iter == temp_iter);
            CHECK(iter.value() == arr[idx]);
            CHECK_THROWS_AS(iter.key(), json_invalid_iterator);
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
    }

    SECTION("others")
    {
        CHECK(json::object({}).begin() != json::array({}).begin());
        CHECK_THROWS_AS((json::iterator{ nullptr }).key(), json_invalid_iterator);
        CHECK_FALSE(json::iterator{ nullptr } == json::iterator{ nullptr });
    }
}
