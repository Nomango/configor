// Copyright (c) 2018-2020 configor - Nomango
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
#include "configor_conversion.hpp"
#include "configor_parser.hpp"
#include "configor_serializer.hpp"
#include "configor_wrapper.hpp"

#include <cstdint>  // std::int64_t
#include <map>      // std::map
#include <string>   // std::string
#include <vector>   // std::vector

namespace configor
{

struct value_tplargs
{
    using boolean_type = bool;

    using integer_type = int64_t;

    using float_type = double;

    using char_type = char;

    template <class _CharTy, class... _Args>
    using string_type = std::basic_string<_CharTy, _Args...>;

    template <class _Kty, class... _Args>
    using array_type = std::vector<_Kty, _Args...>;

    template <class _Kty, class _Ty, class... _Args>
    using object_type = std::map<_Kty, _Ty, _Args...>;

    template <class _Ty>
    using allocator_type = std::allocator<_Ty>;

    template <class _Ty>
    using binder_type = value_binder<_Ty>;
};

struct wvalue_tplargs : value_tplargs
{
    using char_type = wchar_t;
};

using value  = basic_value<value_tplargs>;
using wvalue = basic_value<wvalue_tplargs>;

}  // namespace configor
