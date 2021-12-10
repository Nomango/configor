// Copyright (c) 2021 Nomango

#pragma once

#include "../common/forward_include.h"
#include <configor/json5.hpp>

using namespace configor;

struct u16json5_args : json5_args
{
    using char_type = char16_t;
};

struct u32json5_args : json5_args
{
    using char_type = char32_t;
};

// char16_t
using u16json5 = configor::basic_config<u16json5_args>;
// char32_t
using u32json5 = configor::basic_config<u32json5_args>;
