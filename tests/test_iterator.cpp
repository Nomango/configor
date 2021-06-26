// Copyright (c) 2021 Nomango

#include "common.h"

TEST(test_iterator, string)
{
    json j = "string";

    auto iter = j.begin();
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());
    ASSERT_TRUE(*iter == j);

    ASSERT_THROW(iter.key(), json_invalid_iterator);

    ASSERT_NO_THROW(iter++);
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter != j.begin());
    ASSERT_NO_THROW(iter--);
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());

    ASSERT_THROW(j.end().operator*(), std::out_of_range);

    ASSERT_EQ(std::distance(j.begin(), j.end()), j.size());
    ASSERT_EQ(std::distance(j.rbegin(), j.rend()), j.size());
}

TEST(test_iterator, integer)
{
    json j = 100;

    auto iter = j.begin();
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());
    ASSERT_TRUE(*iter == j);

    ASSERT_THROW(iter.key(), json_invalid_iterator);

    ASSERT_NO_THROW(iter++);
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter != j.begin());
    ASSERT_NO_THROW(iter--);
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());

    ASSERT_THROW(j.end().operator*(), std::out_of_range);

    ASSERT_EQ(std::distance(j.begin(), j.end()), j.size());
    ASSERT_EQ(std::distance(j.rbegin(), j.rend()), j.size());
}

TEST(test_iterator, double)
{
    json j = 100.0;

    auto iter = j.begin();
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());
    ASSERT_TRUE(*iter == j);

    ASSERT_THROW(iter.key(), json_invalid_iterator);

    ASSERT_NO_THROW(iter++);
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter != j.begin());
    ASSERT_NO_THROW(iter--);
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());

    ASSERT_THROW(j.end().operator*(), std::out_of_range);

    ASSERT_EQ(std::distance(j.begin(), j.end()), j.size());
    ASSERT_EQ(std::distance(j.rbegin(), j.rend()), j.size());
}

TEST(test_iterator, boolean)
{
    json j = true;

    auto iter = j.begin();
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());
    ASSERT_TRUE(*iter == j);

    ASSERT_THROW(iter.key(), json_invalid_iterator);

    ASSERT_NO_THROW(iter++);
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter != j.begin());
    ASSERT_NO_THROW(iter--);
    ASSERT_TRUE(iter != j.end());
    ASSERT_TRUE(iter == j.begin());

    ASSERT_THROW(j.end().operator*(), std::out_of_range);

    ASSERT_EQ(std::distance(j.begin(), j.end()), j.size());
    ASSERT_EQ(std::distance(j.rbegin(), j.rend()), j.size());
}

TEST(test_iterator, null)
{
    json j = nullptr;

    auto iter = j.begin();
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter == j.begin());
    ASSERT_THROW(*iter, std::out_of_range);

    ASSERT_THROW(iter.operator*(), std::out_of_range);

    ASSERT_NO_THROW(iter++);
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter == j.begin());
    ASSERT_NO_THROW(iter--);
    ASSERT_TRUE(iter == j.end());
    ASSERT_TRUE(iter == j.begin());

    ASSERT_EQ(std::distance(j.begin(), j.end()), j.size());
    ASSERT_EQ(std::distance(j.rbegin(), j.rend()), j.size());
}

TEST(test_iterator, object)
{
    const json obj = json::object({ { "user", { { "id", 1 }, { "name", "Nomango" } } } });
    for (auto iter = obj.begin(); iter != obj.end(); iter++)
    {
        ASSERT_EQ(iter, obj.find(iter.key()));
        ASSERT_EQ(iter.value(), obj[iter.key()]);
        ASSERT_EQ(iter.value(), *iter);
    }

    for (auto riter = obj.rbegin(); riter != obj.rend(); riter++)
    {
        ASSERT_EQ(riter.value(), obj[riter.key()]);
    }

    ASSERT_THROW(obj.end().operator*(), std::out_of_range);

    ASSERT_TRUE(obj.find("user") != obj.end());
    ASSERT_TRUE(obj.find("missing") == obj.end());

    ASSERT_EQ(std::distance(obj.begin(), obj.end()), obj.size());
    ASSERT_EQ(std::distance(obj.rbegin(), obj.rend()), obj.size());
}

TEST(test_iterator, array)
{
    const json arr = json::array({ 1, 2, 3 });

    size_t idx = 0;
    for (auto iter = arr.begin(); iter != arr.end(); iter++, idx++)
    {
        auto temp_iter = arr.begin();
        std::advance(temp_iter, idx);
        ASSERT_EQ(iter, temp_iter);
        ASSERT_EQ(iter.value(), arr[idx]);
        ASSERT_THROW(iter.key(), json_invalid_iterator);
    }

    idx = 0;
    for (auto elem : arr)
    {
        ASSERT_EQ(elem, arr[idx]);
        idx++;
    }

    idx = arr.size() - 1;
    for (auto riter = arr.rbegin(); riter != arr.rend(); riter++, idx--)
    {
        ASSERT_EQ(riter.value(), arr[idx]);
    }

    ASSERT_THROW(arr.end().operator*(), std::out_of_range);

    ASSERT_EQ(std::distance(arr.begin(), arr.end()), arr.size());
    ASSERT_EQ(std::distance(arr.rbegin(), arr.rend()), arr.size());
}

TEST(test_iterator, others)
{
    ASSERT_TRUE(json::object({}).begin() != json::array({}).begin());
    ASSERT_THROW((json::iterator{ nullptr }).key(), json_invalid_iterator);
    ASSERT_FALSE(json::iterator{ nullptr } == json::iterator{ nullptr });
}
