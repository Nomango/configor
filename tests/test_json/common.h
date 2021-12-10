// Copyright (c) 2021 Nomango

#pragma once

#include <forward_include.h>
#include <configor/json.hpp>

using namespace configor;

struct u16json_args : json_args
{
    using char_type = char16_t;
};

struct u32json_args : json_args
{
    using char_type = char32_t;
};

// char16_t
using u16json = configor::basic_config<u16json_args>;
// char32_t
using u32json = configor::basic_config<u32json_args>;
