// Copyright (c) 2018-2021 jsonxx - Nomango
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
#include "json_encoding.hpp"

#include <cstdint>      // std::int64_t
#include <map>          // std::map
#include <string>       // std::string
#include <type_traits>  // std::false_type, std::true_type, std::remove_cv, std::remove_reference
#include <vector>       // std::vector

namespace jsonxx
{
//
// forward declare
//

template <template <class _Kty, class _Ty, class... _Args> class _ObjectTy = std::map, template <class _Kty, class... _Args> class _ArrayTy = std::vector,
          typename _StringTy = std::string, typename _IntegerTy = std::int64_t, typename _FloatTy = double, typename _BooleanTy = bool,
          template <class _Ty> class _Allocator = std::allocator, template <class _CharTy> class _Encoding = encoding::auto_utf>
class basic_json;

#define DECLARE_BASIC_JSON_TEMPLATE                                                                                                                                            \
    template <template <class _Kty, class _Ty, class... _Args> class _ObjectTy, template <class _Kty, class... _Args> class _ArrayTy, typename _StringTy, typename _IntegerTy, \
              typename _FloatTy, typename _BooleanTy, template <class _Ty> class _Allocator, template <class _CharTy> class _Encoding>

#define DECLARE_BASIC_JSON_TPL_ARGS _ObjectTy, _ArrayTy, _StringTy, _IntegerTy, _FloatTy, _BooleanTy, _Allocator, _Encoding

//
// is_json
//

template <typename>
struct is_json : std::false_type
{
};

DECLARE_BASIC_JSON_TEMPLATE
struct is_json<basic_json<DECLARE_BASIC_JSON_TPL_ARGS>> : std::true_type
{
};

namespace detail
{

template <int p>
struct priority : priority<p - 1>
{
};

template <>
struct priority<0>
{
};

template <typename _Ty>
struct remove_cvref
{
    using type = typename std::remove_cv<typename std::remove_reference<_Ty>::type>::type;
};

}  // namespace detail

}  // namespace jsonxx
