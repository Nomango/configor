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
#include "json_exception.hpp"
#include "json_value.hpp"
#include "json_iterator.hpp"
#include "json_parser.hpp"
#include "json_serializer.hpp"
#include "json_basic.hpp"

namespace jsonxx
{
    using json = basic_json<>;
}

namespace std
{
    template <>
    struct hash<::jsonxx::json>
    {
        std::size_t operator()(const ::jsonxx::json &json) const
        {
            return hash<::jsonxx::json::string_type>{}(json.dump());
        }
    };

    template <>
    inline void swap<::jsonxx::json>(::jsonxx::json &lhs, ::jsonxx::json &rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
