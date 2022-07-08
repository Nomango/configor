// Copyright (c) 2021 Nomango

#include <configor/json.hpp>
#include <iostream>

using namespace configor;

int main(int argc, char** argv)
{
    json::value v{ "sdf" };
    std::cout << "v is string " << (v.is_string()) << std::endl;

    {
        json::value k{ v };
        std::cout << "k is string " << (k.is_string()) << std::endl;
        std::cout << "v is string " << (v.is_string()) << std::endl;
    }

    {
        json::value j{ std::move(v) };
        std::cout << "j is string " << (j.is_string()) << std::endl;
        std::cout << "v is string " << (v.is_string()) << std::endl;
    }

    {
        wjson::value w{ L"sdf" };
        std::cout << "w is string " << (w.is_string()) << std::endl;
    }
    return 0;
}
