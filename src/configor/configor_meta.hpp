#pragma once

#include <cstdint>  // std::int64_t
#include <map>      // std::map
#include <string>   // std::basic_string
#include <vector>   // std::vector

namespace configor
{

enum class configor_type
{
    null,
    boolean,
    number_integer,
    number_float,
    string,
    array,
    object,
};

inline const char* to_string(configor_type t) noexcept
{
    switch (t)
    {
    case configor_type::object:
        return "object";
    case configor_type::array:
        return "array";
    case configor_type::string:
        return "string";
    case configor_type::number_integer:
        return "integer";
    case configor_type::number_float:
        return "float";
    case configor_type::boolean:
        return "boolean";
    case configor_type::null:
        return "null";
    }
    return "unknown";
}

struct configor_tpl_args
{
    using boolean_type = bool;

    using integer_type = int64_t;

    using float_type = double;

    using char_type = char;

    template <class CharT>
    using string_type = std::basic_string<CharT>;

    template <class T>
    using array_type = std::vector<T>;

    template <class K, class T>
    using object_type = std::map<K, T>;

    template <class T>
    using allocator_type = std::allocator<T>;
};

struct wconfigor_tpl_args : configor_tpl_args
{
    using char_type = wchar_t;
};

template <class TplArgs>
class basic_configor;

}  // namespace configor
