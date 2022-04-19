// Copyright (c) 2021-2022 configor - Nomango
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
#include "config.hpp"

#include <iomanip> // std::setprecision, std::right, std::noshowbase

namespace configor {

namespace detail {

template <typename ValueT>
class json_parser;

template <typename ValueT>
class json_serializer;

struct json_serialization_args {
    template <typename ValueT>
    using parser_type = json_parser<ValueT>;

    template <typename ValueT>
    using serializer_type = json_serializer<ValueT>;
};

} // namespace detail

template <class ValueArgs>
using basic_json = detail::basic_config<ValueArgs, detail::json_serialization_args>;

using json  = basic_json<config_value_args>;
using wjson = basic_json<wconfig_value_args>;

} // namespace configor
