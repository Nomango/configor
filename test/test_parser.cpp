// Copyright (c) 2019 Nomango

#include "test_parser.h"
#include "nomango/json.hpp"
#include <array>
#include <fstream>

using namespace nomango;

void test_parser()
{
	{
		auto j = json::parse(L"{ \"happy\": true, \"pi\": 3.141 }");
	}

	{
		std::array<std::wstring, 5> files = {
			L"data/json.org/1.json",
			L"data/json.org/2.json",
			L"data/json.org/3.json",
			L"data/json.org/4.json",
			L"data/json.org/5.json",
		};

		for (const auto& file : files)
		{
			// read a JSON file
			std::wifstream ifs(file);

			json j;
			ifs >> j;
		}
	}
}
