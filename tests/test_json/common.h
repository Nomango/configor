// Copyright (c) 2021 Nomango

#pragma once

#include "ctype.h"

#include <catch2/catch.hpp>
#include <configor/json.hpp>

using namespace configor;

#define COMBINE(A, B) A##B
#define WIDE(STR) COMBINE(L, STR)

#define STR_ANY(ANY) #ANY
#define STR(ANY) STR_ANY(ANY)

#define RAW_STR "我是地球🌍"
#define QUOTE_STR "\"我是地球🌍\""
#define ESCAPED_STR "\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\""
