#pragma once


enum class token_t {
    object_begin,
    object_end,
    object_key,
    object_value,

    array_begin,
    array_end,
    array_value,

    value_string,
    value_integer,
    value_float,
    value_bool,
    value_null,
};
