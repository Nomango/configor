// Copyright (c) 2018-2020 jsonxx - Nomango
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
#include "json_basic.hpp"

namespace jsonxx
{

using json  = basic_json<>;
using wjson = basic_json<std::map, std::vector, std::wstring>;

inline void swap(::jsonxx::json& lhs, ::jsonxx::json& rhs)
{
    lhs.swap(rhs);
}

inline void swap(::jsonxx::wjson& lhs, ::jsonxx::wjson& rhs)
{
    lhs.swap(rhs);
}

}  // namespace jsonxx

namespace std
{
template <>
struct hash<::jsonxx::json>
{
    using argument_type = ::jsonxx::json;
    using result_type   = size_t;

    result_type operator()(argument_type const& json) const
    {
        return hash<argument_type::string_type>{}(json.dump());
    }
};

template <>
struct hash<::jsonxx::wjson>
{
    using argument_type = ::jsonxx::wjson;
    using result_type   = size_t;

    result_type operator()(argument_type const& json) const
    {
        return hash<argument_type::string_type>{}(json.dump());
    }
};
}  // namespace std
