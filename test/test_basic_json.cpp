// Copyright (c) 2019 Nomango

#include "test_basic_json.h"
#include "nomango/json.hpp"
#include <iostream>

using namespace nomango;

void test_basic_json()
{
	{
		json j;

		// add a number that is stored as double (note the implicit conversion of j to an object)
		j[L"pi"] = 3.141;

		// add a Boolean that is stored as bool
		j[L"happy"] = true;

		// add a string that is stored as std::string
		j[L"name"] = L"Nomango";

		// add another null object by passing nullptr
		j[L"nothing"] = nullptr;

		// add an object inside the object
		j[L"answer"][L"everything"] = 42;

		// add an array that is stored as std::vector (using an initializer list)
		j[L"list"] = { 1, 0, 2 };

		// add another object (using an initializer list of pairs)
		j[L"object"] = { {L"currency", "USD"}, {L"value", 42.99} };
	}

	json j = {
		{L"pi", 3.141},
		{L"happy", true},
		{L"name", L"Nomango"},
		{L"nothing", nullptr},
		{L"answer", {
				{L"everything", 42}
		}},
		{L"list", {1, 0, 2}},
		{L"object", {
			{L"currency", L"USD"},
			{L"value", 42.99}
		}}
	};

	{
		// for-each
		for (const auto& v : j)
		{
			(void)v.dump();
		}
	}

	{
		// iterator
		for (auto iter = j.begin(); iter != j.end(); iter++)
		{
			(void)iter->dump();
		}
	}
}
