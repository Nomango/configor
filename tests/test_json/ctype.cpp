// Copyright (c) 2021 Nomango

#include "ctype.h"

#if defined(__clang__)

std::locale::id std::ctype<char16_t>::id;
std::locale::id std::ctype<char32_t>::id;

#endif
