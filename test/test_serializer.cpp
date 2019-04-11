// Copyright (c) 2019 Nomango

#include "test_serializer.h"
#include "nomango/json.hpp"
#include <fstream>
#include <iomanip>

using namespace nomango;

void test_serializer()
{
	json j = {
		{L"pi", 3.141},
		{L"happy", true},
		{L"name", "Nomango"},
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
		// write prettified JSON to another file
		std::wofstream o(L"data/output/pretty.json");
		o << std::setw(4) << j << std::endl;
	}

	{
		std::wstring serialized_string = j.dump();
	}

	{
		std::wstring serialized_string = j.dump(4, ' ');
	}
}
