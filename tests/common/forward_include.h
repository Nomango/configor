// Copyright (c) 2021 Nomango

#pragma once

#include "ctype.h"

#include <catch2/catch.hpp>

#define COMBINE(A, B) A##B
#define WIDE(STR) COMBINE(L, STR)
#define U16(STR) COMBINE(u, STR)
#define U32(STR) COMBINE(U, STR)

#define STR_ANY(ANY) #ANY
#define STR(ANY) STR_ANY(ANY)
