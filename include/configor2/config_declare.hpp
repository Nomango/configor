// Copyright (c) 2018-2021 configor - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include "config_exception.hpp"
#include "meta.hpp"

#include <cstdint>     // int64_t
#include <map>         // std::map
#include <memory>      // std::allocator
#include <string>      // std::basic_string
#include <type_traits> // std::char_traits
#include <vector>      // std::vector

namespace configor {

struct config_value_args {
    using boolean_type = bool;

    using integer_type = int64_t;

    using float_type = double;

    using char_type = char;

    template <class T>
    using allocator_type = std::allocator<T>;

    template <class CharT, class Traits = std::char_traits<CharT>, class Allocator = allocator_type<CharT>>
    using string_type = std::basic_string<CharT, Traits, Allocator>;

    template <class T, class Allocator = allocator_type<T>>
    using array_type = std::vector<T, Allocator>;

    template <class Key, class T, class Compare = std::less<Key>,
              class Allocator = allocator_type<std::pair<const Key, T>>>
    using object_type = std::map<Key, T, Compare, Allocator>;
};

struct wconfig_value_args : config_value_args {
    using char_type = wchar_t;
};

namespace detail {

template <class ValueArgs>
class config_value;

//
// is_config_value
//

template <class>
struct is_config_value {
    static constexpr bool value = false;
};

template <class ValueArgs>
struct is_config_value<config_value<ValueArgs>> {
    static constexpr bool value = true;
};

// config_value_type

enum class config_value_type {
    null,
    boolean,
    number_integer,
    number_float,
    string,
    array,
    object,
};

inline const char* to_string(config_value_type t) noexcept {
    switch (t) {
    case config_value_type::object:
        return "object";
    case config_value_type::array:
        return "array";
    case config_value_type::string:
        return "string";
    case config_value_type::number_integer:
        return "integer";
    case config_value_type::number_float:
        return "float";
    case config_value_type::boolean:
        return "boolean";
    case config_value_type::null:
        return "null";
    }
    return "unknown";
}

// token_type

enum class token_type {
    none,

    value_null,
    value_boolean,
    value_integer,
    value_float,
    value_string,

    begin_array,
    end_array,
    begin_object,
    end_object,

    eof
};

inline const char* to_string(token_type token) {
    switch (token) {
    case token_type::none:
        return "none";
    case token_type::value_null:
        return "value_null";
    case token_type::value_boolean:
        return "value_boolean";
    case token_type::value_integer:
        return "value_integer";
    case token_type::value_float:
        return "value_float";
    case token_type::value_string:
        return "value_string";
    case token_type::array_begin:
        return "array_begin";
    case token_type::array_end:
        return "array_end";
    case token_type::object_begin:
        return "object_begin";
    case token_type::object_end:
        return "object_end";
    case token_type::eof:
        return "EOF";
    }
    return "unknown";
}

// nearly_equal

template <typename T>
bool nearly_equal(T a, T b) {
    return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
}
} // namespace detail

} // namespace configor
