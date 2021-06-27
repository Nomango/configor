// Copyright (c) 2021 Nomango

#include "ctype.h"

#if defined(__clang__)

std::locale::id std::ctype<char16_t>::id;

bool std::ctype<char16_t>::do_is(mask m, char_type c) const
{
    const auto& ct = std::use_facet<std::ctype<char>>(std::locale{});
    return ct.is(m, static_cast<char>(c));
}

std::locale::id std::ctype<char32_t>::id;

bool std::ctype<char32_t>::do_is(mask m, char_type c) const
{
    const auto& ct = std::use_facet<std::ctype<char>>(std::locale{});
    return ct.is(m, static_cast<char>(c));
}

#endif
