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
#include "configor_encoding.hpp"

#include <cstdint>      // std::int64_t
#include <map>          // std::map
#include <string>       // std::string
#include <type_traits>  // std::false_type, std::true_type, std::remove_cv, std::remove_reference
#include <vector>       // std::vector

namespace configor
{

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

template <typename _Ty, typename... _Args>
struct get_last
{
    using type = typename get_last<_Args...>::type;
};

template <typename _Ty>
struct get_last<_Ty>
{
    using type = _Ty;
};

}  // namespace detail

//
// forward declare
//

struct config_args
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

    template <class _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using lexer_type = void;

    template <class _ConfTy>
    using lexer_args_type = void;

    template <class _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using serializer_type = void;

    template <class _ConfTy>
    using serializer_args_type = void;

    template <typename _CharTy>
    using default_encoding = encoding::ignore<_CharTy>;
};

struct wconfig_args : config_args
{
    using char_type = wchar_t;
};

template <typename _Args = config_args>
class basic_config;

//
// is_config
//

template <typename>
struct is_config : std::false_type
{
};

template <typename _Args>
struct is_config<basic_config<_Args>> : std::true_type
{
};

}  // namespace configor
