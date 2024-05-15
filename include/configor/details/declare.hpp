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
#include <type_traits>  // std::false_type, std::true_type, std::remove_cv, std::remove_reference

namespace configor
{

//
// forward declare
//

template <typename _Ty>
class value_binder;

template <typename _Args>
class basic_value;

//
// is_value
//

template <typename>
struct is_value : std::false_type
{
};

template <typename _Args>
struct is_value<basic_value<_Args>> : std::true_type
{
};

//
// value_constant
//

struct value_constant
{
    enum type
    {
        null,
        integer,
        floating,
        string,
        array,
        object,
        boolean,
    };
};

inline const char* to_string(value_constant::type t) noexcept
{
    switch (t)
    {
    case value_constant::type::object:
        return "object";
    case value_constant::type::array:
        return "array";
    case value_constant::type::string:
        return "string";
    case value_constant::type::integer:
        return "integer";
    case value_constant::type::floating:
        return "float";
    case value_constant::type::boolean:
        return "boolean";
    case value_constant::type::null:
        return "null";
    }
    return "unknown";
}

namespace detail
{

// priority

template <int p>
struct priority : priority<p - 1>
{
};

template <>
struct priority<0>
{
};

// remove_cvref

template <typename _Ty>
struct remove_cvref
{
    using type = typename std::remove_cv<typename std::remove_reference<_Ty>::type>::type;
};

// always_void

template <typename _Ty>
struct always_void
{
    using type = void;
};

// exact_detect

template <class _Void, template <class...> class _Op, class... _Args>
struct detect_impl
{
    struct dummy
    {
        dummy()             = delete;
        ~dummy()            = delete;
        dummy(const dummy&) = delete;
    };

    using type = dummy;

    static constexpr bool value = false;
};

template <template <class...> class _Op, class... _Args>
struct detect_impl<typename always_void<_Op<_Args...>>::type, _Op, _Args...>
{
    using type = _Op<_Args...>;

    static constexpr bool value = true;
};

template <class _Expected, template <class...> class _Op, class... _Args>
using exact_detect = std::is_same<_Expected, typename detect_impl<void, _Op, _Args...>::type>;

template <template <class...> class _Op, class... _Args>
using is_detected = std::integral_constant<bool, detect_impl<void, _Op, _Args...>::value>;

// static_const

template <typename _Ty>
struct static_const
{
    static constexpr _Ty value = {};
};

template <typename _Ty>
constexpr _Ty static_const<_Ty>::value;

// to value

template <typename _ValTy, typename _Ty, typename _Void = void>
struct has_to_value : std::false_type
{
};

template <typename _ValTy, typename _Ty>
struct has_to_value<_ValTy, _Ty, typename std::enable_if<!is_value<_Ty>::value>::type>
{
private:
    using binder_type = typename _ValTy::template binder_type<_Ty>;

    template <typename _UTy, typename... _Args>
    using to_config_fn = decltype(_UTy::to_value(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<void, to_config_fn, binder_type, _ValTy&, _Ty>::value;
};

// from value

template <typename _ValTy, typename _Ty, typename _Void = void>
struct has_from_value : std::false_type
{
};

template <typename _ValTy, typename _Ty>
struct has_from_value<_ValTy, _Ty, typename std::enable_if<!is_value<_Ty>::value>::type>
{
private:
    using binder_type = typename _ValTy::template binder_type<_Ty>;

    template <typename _UTy, typename... _Args>
    using from_config_fn = decltype(_UTy::from_value(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<void, from_config_fn, binder_type, _ValTy, _Ty&>::value;
};

template <typename _ValTy, typename _Ty, typename _Void = void>
struct has_non_default_from_value : std::false_type
{
};

template <typename _ValTy, typename _Ty>
struct has_non_default_from_value<_ValTy, _Ty, typename std::enable_if<!is_value<_Ty>::value>::type>
{
private:
    using binder_type = typename _ValTy::template binder_type<_Ty>;

    template <typename _UTy, typename... _Args>
    using from_config_fn = decltype(_UTy::from_value(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<_Ty, from_config_fn, binder_type, _ValTy>::value;
};

// getable

template <typename _ValTy, typename _Ty, typename _Void = void>
struct is_value_getable : std::false_type
{
};

template <typename _ValTy, typename _Ty>
struct is_value_getable<_ValTy, _Ty, typename std::enable_if<!is_value<_Ty>::value>::type>
{
private:
    template <typename _UTy, typename... _Args>
    using get_fn = decltype(std::declval<_UTy>().template get<_Args...>());

public:
    static constexpr bool value = exact_detect<_Ty, get_fn, _ValTy, _Ty>::value;
};

}  // namespace detail

}  // namespace configor
