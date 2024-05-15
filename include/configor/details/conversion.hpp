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
#include "stream.hpp"
#include "value.hpp"

#include <array>          // std::array
#include <deque>          // std::deque
#include <forward_list>   // std::forward_list
#include <list>           // std::list
#include <map>            // std::map
#include <memory>         // std::unique_ptr, std::shared_ptr, std::make_shared
#include <queue>          // std::queue
#include <set>            // std::set
#include <type_traits>    // std::enable_if, std::is_same, std::false_type, std::true_type, std::is_void
#include <unordered_map>  // std::unordered_map
#include <unordered_set>  // std::unordered_set
#include <utility>        // std::forward, std::declval
#include <vector>         // std::vector

namespace configor
{
namespace detail
{
namespace
{
inline configor_type_error make_conversion_error(value_constant::type t, value_constant::type want)
{
    fast_ostringstream ss;
    ss << "cannot convert type '" << to_string(t) << "' to type '" << to_string(want) << "' (implicitly)";
    return configor_type_error(ss.str());
}
}  // namespace

// to_value functions

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ValTy::boolean_type>::value, int>::type = 0>
void to_value(_ValTy& c, _Ty v)
{
    value_constructor<_ValTy>::template reset<value_constant::boolean>(c, v);
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<
              std::is_integral<_Ty>::value && !std::is_same<_Ty, typename _ValTy::boolean_type>::value, int>::type = 0>
void to_value(_ValTy& c, _Ty v)
{
    value_constructor<_ValTy>::template reset<value_constant::integer>(c,
                                                                       static_cast<typename _ValTy::integer_type>(v));
}

template <typename _ValTy, typename _Ty, typename std::enable_if<std::is_floating_point<_Ty>::value, int>::type = 0>
void to_value(_ValTy& c, _Ty v)
{
    value_constructor<_ValTy>::template reset<value_constant::floating>(c, static_cast<typename _ValTy::float_type>(v));
}

template <typename _ValTy>
void to_value(_ValTy& c, const typename _ValTy::string_type& v)
{
    value_constructor<_ValTy>::template reset<value_constant::string>(c, v);
}

template <typename _ValTy>
void to_value(_ValTy& c, typename _ValTy::string_type&& v)
{
    value_constructor<_ValTy>::template reset<value_constant::string>(c, std::move(v));
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<typename _ValTy::string_type, _Ty>::value
                                      && !std::is_same<_Ty, typename _ValTy::string_type>::value,
                                  int>::type = 0>
void to_value(_ValTy& c, const _Ty& v)
{
    value_constructor<_ValTy>::template reset<value_constant::string>(c, v);
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ValTy::array_type>::value, int>::type = 0>
void to_value(_ValTy& c, _Ty& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c, v);
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ValTy::object_type>::value, int>::type = 0>
void to_value(_ValTy& c, _Ty& v)
{
    value_constructor<_ValTy>::template reset<value_constant::object>(c, v);
}

// from_value functions

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ValTy::boolean_type>::value, int>::type = 0>
void from_value(const _ValTy& c, _Ty& v)
{
    if (!c.is_bool())
        throw make_conversion_error(c.type(), value_constant::boolean);
    v = c.data().boolean;
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<
              std::is_integral<_Ty>::value && !std::is_same<_Ty, typename _ValTy::boolean_type>::value, int>::type = 0>
void from_value(const _ValTy& c, _Ty& v)
{
    if (!c.is_integer())
        throw make_conversion_error(c.type(), value_constant::integer);
    v = static_cast<_Ty>(c.data().integer);
}

template <typename _ValTy, typename _Ty, typename std::enable_if<std::is_floating_point<_Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, _Ty& v)
{
    if (!c.is_floating())
        throw make_conversion_error(c.type(), value_constant::floating);
    v = static_cast<_Ty>(c.data().floating);
}

template <typename _ValTy>
void from_value(const _ValTy& c, typename _ValTy::string_type& v)
{
    if (!c.is_string())
        throw make_conversion_error(c.type(), value_constant::string);
    v = *c.data().string;
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_Ty, typename _ValTy::string_type>::value
                                      && !std::is_same<_Ty, typename _ValTy::string_type>::value,
                                  int>::type = 0>
void from_value(const _ValTy& c, _Ty& v)
{
    if (!c.is_string())
        throw make_conversion_error(c.type(), value_constant::string);
    v = *c.data().string;
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ValTy::array_type>::value, int>::type = 0>
void from_value(const _ValTy& c, _Ty& v)
{
    if (c.is_null())
    {
        v.clear();
        return;
    }
    if (!c.is_array())
        throw make_conversion_error(c.type(), value_constant::array);
    v.assign((*c.data().vector).begin(), (*c.data().vector).end());
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ValTy::object_type>::value, int>::type = 0>
void from_value(const _ValTy& c, _Ty& v)
{
    if (c.is_null())
    {
        v.clear();
        return;
    }
    if (!c.is_object())
        throw make_conversion_error(c.type(), value_constant::object);
    v = *c.data().object;
}

// c-style array

template <
    typename _ValTy, typename _Ty, size_t _Num,
    typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value
                                && !std::is_constructible<typename _ValTy::string_type, const _Ty (&)[_Num]>::value,
                            int>::type = 0>
void to_value(_ValTy& c, const _Ty (&v)[_Num])
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    for (size_t i = 0; i < _Num; i++)
    {
        c[i] = v[i];
    }
}

template <
    typename _ValTy, typename _Ty, size_t _Num,
    typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value
                                && !std::is_constructible<typename _ValTy::string_type, const _Ty (&)[_Num]>::value,
                            int>::type = 0>
void from_value(const _ValTy& c, _Ty (&v)[_Num])
{
    for (size_t i = 0; i < c.size() && i < _Num; i++)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

// other conversions

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::unique_ptr<_Ty>& v)
{
    if (v != nullptr)
    {
        c = *v;
    }
    else
    {
        c = nullptr;
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::unique_ptr<_Ty>& v)
{
    if (c.is_null())
    {
        v = nullptr;
    }
    else
    {
        v.reset(new _Ty(c.template get<_Ty>()));
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::shared_ptr<_Ty>& v)
{
    if (v != nullptr)
    {
        c = *v;
    }
    else
    {
        c = nullptr;
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::shared_ptr<_Ty>& v)
{
    if (c.is_null())
    {
        v = nullptr;
    }
    else
    {
        v = std::make_shared<_Ty>(c.template get<_Ty>());
    }
}

template <typename _ValTy, typename _Ty, size_t _Num,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::array<_Ty, _Num>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    for (size_t i = 0; i < _Num; i++)
    {
        c[i] = v.at(i);
    }
}

template <typename _ValTy, typename _Ty, size_t _Num,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::array<_Ty, _Num>& v)
{
    for (size_t i = 0; i < c.size() && i < _Num; i++)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::vector<_Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    for (size_t i = 0; i < v.size(); ++i)
    {
        c[i] = v.at(i);
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::vector<_Ty>& v)
{
    v.resize(c.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::deque<_Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    for (size_t i = 0; i < v.size(); ++i)
    {
        c[i] = v.at(i);
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::deque<_Ty>& v)
{
    v.resize(c.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::list<_Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    auto iter = v.begin();
    for (size_t i = 0; iter != v.end(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::list<_Ty>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.push_back(c[i].template get<_Ty>());
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::forward_list<_Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    auto iter = v.begin();
    for (size_t i = 0; iter != v.end(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::forward_list<_Ty>& v)
{
    v.clear();
    size_t size = c.size();
    for (size_t i = 0; i < size; i++)
    {
        v.push_front(c[size - i - 1].template get<_Ty>());
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::set<_Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    auto iter = v.begin();
    for (size_t i = 0; i < v.size(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::set<_Ty>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.insert(c[i].template get<_Ty>());
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value, int>::type = 0>
void to_value(_ValTy& c, const std::unordered_set<_Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::array>(c);
    auto iter = v.begin();
    for (size_t i = 0; i < v.size(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ValTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value, int>::type = 0>
void from_value(const _ValTy& c, std::unordered_set<_Ty>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.insert(c[i].template get<_Ty>());
    }
}

template <typename _ValTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value
                                      && std::is_constructible<typename _ValTy::string_type, _KeyTy>::value,
                                  int>::type = 0>
void to_value(_ValTy& c, const std::map<_KeyTy, _Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::object>(c);
    for (const auto& p : v)
    {
        c[p.first] = p.second;
    }
}

template <typename _ValTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value
                                      && std::is_constructible<_KeyTy, typename _ValTy::string_type>::value,
                                  int>::type = 0>
void from_value(const _ValTy& c, std::map<_KeyTy, _Ty>& v)
{
    for (auto iter = c.cbegin(); iter != c.cend(); iter++)
    {
        v.emplace(iter.key(), iter.value().template get<_Ty>());
    }
}

template <typename _ValTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ValTy, _Ty>::value
                                      && std::is_constructible<typename _ValTy::string_type, _KeyTy>::value,
                                  int>::type = 0>
void to_value(_ValTy& c, const std::unordered_map<_KeyTy, _Ty>& v)
{
    value_constructor<_ValTy>::template reset<value_constant::object>(c);
    for (const auto& p : v)
    {
        c[p.first] = p.second;
    }
}

template <typename _ValTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<detail::is_value_getable<_ValTy, _Ty>::value
                                      && std::is_constructible<_KeyTy, typename _ValTy::string_type>::value,
                                  int>::type = 0>
void from_value(const _ValTy& c, std::unordered_map<_KeyTy, _Ty>& v)
{
    for (auto iter = c.cbegin(); iter != c.cend(); iter++)
    {
        v.emplace(iter.key(), iter.value().template get<_Ty>());
    }
}

//
// to_value & from_value function object
// Explanation: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html
//
struct to_config_fn
{
    template <typename _ValTy, typename _Ty>
    auto operator()(_ValTy& c, _Ty&& v) const noexcept(noexcept(to_value(c, std::forward<_Ty>(v))))
        -> decltype(to_value(c, std::forward<_Ty>(v)))
    {
        return to_value(c, std::forward<_Ty>(v));
    }
};

struct from_config_fn
{
    template <typename _ValTy, typename _Ty>
    auto operator()(const _ValTy& c, _Ty&& v) const noexcept(noexcept(from_value(c, std::forward<_Ty>(v))))
        -> decltype(from_value(c, std::forward<_Ty>(v)))
    {
        return from_value(c, std::forward<_Ty>(v));
    }
};

}  // namespace detail

namespace
{
constexpr auto const& to_value   = detail::static_const<detail::to_config_fn>::value;
constexpr auto const& from_value = detail::static_const<detail::from_config_fn>::value;
}  // namespace

//
// value_binder
//

template <typename _Ty>
class value_binder
{
public:
    template <typename _ValTy, typename _UTy = _Ty>
    static auto to_value(_ValTy& c, _UTy&& v) noexcept(noexcept(::configor::to_value(c, std::forward<_UTy>(v))))
        -> decltype(::configor::to_value(c, std::forward<_UTy>(v)))
    {
        return ::configor::to_value(c, std::forward<_UTy>(v));
    }

    template <typename _ValTy, typename _UTy = _Ty>
    static auto from_value(_ValTy&& c, _UTy& v) noexcept(noexcept(::configor::from_value(std::forward<_ValTy>(c), v)))
        -> decltype(::configor::from_value(std::forward<_ValTy>(c), v))
    {
        return ::configor::from_value(std::forward<_ValTy>(c), v);
    }
};

}  // namespace configor

// __CONFIGOR_EXPAND is a solution for this question:
// https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion
#define __CONFIGOR_EXPAND(x) x

#define __CONFIGOR_COMBINE_INNER(a, b) a##b
#define __CONFIGOR_COMBINE(a, b) __CONFIGOR_COMBINE_INNER(a, b)

#define __CONFIGOR_GET_ARG_MAX50(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                                 _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                                 _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, ARG, ...)       \
    ARG

#define __CONFIGOR_COUNT_ARGS_MAX50(...)                                                                               \
    __CONFIGOR_EXPAND(__CONFIGOR_GET_ARG_MAX50(__VA_ARGS__, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37,    \
                                               36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, \
                                               18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define __CONFIGOR_CALL_OVERLOAD(name, ...) \
    __CONFIGOR_EXPAND(__CONFIGOR_COMBINE(name, __CONFIGOR_COUNT_ARGS_MAX50(__VA_ARGS__))(__VA_ARGS__))

#define __CONFIGOR_PASTE(parse1, func, ...)                                                                 \
    __CONFIGOR_EXPAND(__CONFIGOR_GET_ARG_MAX50(                                                             \
        __VA_ARGS__, __CONFIGOR_PASTE50, __CONFIGOR_PASTE49, __CONFIGOR_PASTE48, __CONFIGOR_PASTE47,        \
        __CONFIGOR_PASTE46, __CONFIGOR_PASTE45, __CONFIGOR_PASTE44, __CONFIGOR_PASTE43, __CONFIGOR_PASTE42, \
        __CONFIGOR_PASTE41, __CONFIGOR_PASTE40, __CONFIGOR_PASTE39, __CONFIGOR_PASTE38, __CONFIGOR_PASTE37, \
        __CONFIGOR_PASTE36, __CONFIGOR_PASTE35, __CONFIGOR_PASTE34, __CONFIGOR_PASTE33, __CONFIGOR_PASTE32, \
        __CONFIGOR_PASTE31, __CONFIGOR_PASTE30, __CONFIGOR_PASTE29, __CONFIGOR_PASTE28, __CONFIGOR_PASTE27, \
        __CONFIGOR_PASTE26, __CONFIGOR_PASTE25, __CONFIGOR_PASTE24, __CONFIGOR_PASTE23, __CONFIGOR_PASTE22, \
        __CONFIGOR_PASTE21, __CONFIGOR_PASTE20, __CONFIGOR_PASTE19, __CONFIGOR_PASTE18, __CONFIGOR_PASTE17, \
        __CONFIGOR_PASTE16, __CONFIGOR_PASTE15, __CONFIGOR_PASTE14, __CONFIGOR_PASTE13, __CONFIGOR_PASTE12, \
        __CONFIGOR_PASTE11, __CONFIGOR_PASTE10, __CONFIGOR_PASTE9, __CONFIGOR_PASTE8, __CONFIGOR_PASTE7,    \
        __CONFIGOR_PASTE6, __CONFIGOR_PASTE5, __CONFIGOR_PASTE4, __CONFIGOR_PASTE3, __CONFIGOR_PASTE2,      \
        parse1)(parse1, func, __VA_ARGS__))

#define __CONFIGOR_COMBINE_PASTE1(parse1, func, _1) func##_1
#define __CONFIGOR_CALL_PASTE1(parse1, func, _1) func(_1)

#define __CONFIGOR_PASTE2(parse1, func, _1, _2) parse1(parse1, func, _1) parse1(parse1, func, _2)
#define __CONFIGOR_PASTE3(parse1, func, _1, _2, _3) parse1(parse1, func, _1) __CONFIGOR_PASTE2(parse1, func, _2, _3)
#define __CONFIGOR_PASTE4(parse1, func, _1, _2, _3, _4) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE3(parse1, func, _2, _3, _4)
#define __CONFIGOR_PASTE5(parse1, func, _1, _2, _3, _4, _5) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE4(parse1, func, _2, _3, _4, _5)
#define __CONFIGOR_PASTE6(parse1, func, _1, _2, _3, _4, _5, _6) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE5(parse1, func, _2, _3, _4, _5, _6)
#define __CONFIGOR_PASTE7(parse1, func, _1, _2, _3, _4, _5, _6, _7) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE6(parse1, func, _2, _3, _4, _5, _6, _7)
#define __CONFIGOR_PASTE8(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE7(parse1, func, _2, _3, _4, _5, _6, _7, _8)
#define __CONFIGOR_PASTE9(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE8(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9)
#define __CONFIGOR_PASTE10(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE9(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10)
#define __CONFIGOR_PASTE11(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE10(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)
#define __CONFIGOR_PASTE12(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE11(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)
#define __CONFIGOR_PASTE13(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE12(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)
#define __CONFIGOR_PASTE14(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
    parse1(parse1, func, _1) __CONFIGOR_PASTE13(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)
#define __CONFIGOR_PASTE15(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
    parse1(parse1, func, _1)                                                                               \
        __CONFIGOR_PASTE14(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)
#define __CONFIGOR_PASTE16(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
    parse1(parse1, func, _1)                                                                                    \
        __CONFIGOR_PASTE15(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)
#define __CONFIGOR_PASTE17(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
    parse1(parse1, func, _1)                                                                                         \
        __CONFIGOR_PASTE16(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17)
#define __CONFIGOR_PASTE18(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, \
                           _18)                                                                                      \
    parse1(parse1, func, _1)                                                                                         \
        __CONFIGOR_PASTE17(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18)
#define __CONFIGOR_PASTE19(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19)                                                                                   \
    parse1(parse1, func, _1) __CONFIGOR_PASTE18(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19)
#define __CONFIGOR_PASTE20(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20)                                                                              \
    parse1(parse1, func, _1) __CONFIGOR_PASTE19(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20)
#define __CONFIGOR_PASTE21(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21)                                                                         \
    parse1(parse1, func, _1) __CONFIGOR_PASTE20(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21)
#define __CONFIGOR_PASTE22(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22)                                                                    \
    parse1(parse1, func, _1) __CONFIGOR_PASTE21(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22)
#define __CONFIGOR_PASTE23(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23)                                                               \
    parse1(parse1, func, _1) __CONFIGOR_PASTE22(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23)
#define __CONFIGOR_PASTE24(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24)                                                          \
    parse1(parse1, func, _1) __CONFIGOR_PASTE23(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24)
#define __CONFIGOR_PASTE25(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25)                                                     \
    parse1(parse1, func, _1) __CONFIGOR_PASTE24(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25)
#define __CONFIGOR_PASTE26(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26)                                                \
    parse1(parse1, func, _1) __CONFIGOR_PASTE25(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26)
#define __CONFIGOR_PASTE27(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27)                                           \
    parse1(parse1, func, _1) __CONFIGOR_PASTE26(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27)
#define __CONFIGOR_PASTE28(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28)                                      \
    parse1(parse1, func, _1) __CONFIGOR_PASTE27(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28)
#define __CONFIGOR_PASTE29(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29)                                \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE28(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29)
#define __CONFIGOR_PASTE30(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)                           \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE29(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)
#define __CONFIGOR_PASTE31(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)                      \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE30(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)
#define __CONFIGOR_PASTE32(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)                 \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE31(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)
#define __CONFIGOR_PASTE33(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)            \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE32(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)
#define __CONFIGOR_PASTE34(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)       \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE33(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)
#define __CONFIGOR_PASTE35(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)  \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE34(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)
#define __CONFIGOR_PASTE36(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36)                                                                                       \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE35(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36)
#define __CONFIGOR_PASTE37(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37)                                                                                   \
    parse1(parse1, func, _1) __CONFIGOR_PASTE36(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
                                                _29, _30, _31, _32, _33, _34, _35, _36, _37)
#define __CONFIGOR_PASTE38(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38)                                                                              \
    parse1(parse1, func, _1) __CONFIGOR_PASTE37(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
                                                _29, _30, _31, _32, _33, _34, _35, _36, _37, _38)
#define __CONFIGOR_PASTE39(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38, _39)                                                                         \
    parse1(parse1, func, _1) __CONFIGOR_PASTE38(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
                                                _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39)
#define __CONFIGOR_PASTE40(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38, _39, _40)                                                                    \
    parse1(parse1, func, _1) __CONFIGOR_PASTE39(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
                                                _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40)
#define __CONFIGOR_PASTE41(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38, _39, _40, _41)                                                               \
    parse1(parse1, func, _1) __CONFIGOR_PASTE40(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
                                                _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41)
#define __CONFIGOR_PASTE42(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38, _39, _40, _41, _42)                                                          \
    parse1(parse1, func, _1) __CONFIGOR_PASTE41(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                                _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
                                                _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42)
#define __CONFIGOR_PASTE43(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38, _39, _40, _41, _42, _43)                                                     \
    parse1(parse1, func, _1) __CONFIGOR_PASTE42(                                                                       \
        parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, \
        _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43)
#define __CONFIGOR_PASTE44(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,   \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,   \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44)                                                \
    parse1(parse1, func, _1) __CONFIGOR_PASTE43(                                                                       \
        parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, \
        _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44)
#define __CONFIGOR_PASTE45(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44, _45)                                          \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE44(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                           _37, _38, _39, _40, _41, _42, _43, _44, _45)
#define __CONFIGOR_PASTE46(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46)                                     \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE45(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                           _37, _38, _39, _40, _41, _42, _43, _44, _45, _46)
#define __CONFIGOR_PASTE47(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)                                \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE46(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                           _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)
#define __CONFIGOR_PASTE48(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)                           \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE47(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                           _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)
#define __CONFIGOR_PASTE49(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)                      \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE48(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                           _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)
#define __CONFIGOR_PASTE50(parse1, func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,  \
                           _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35,  \
                           _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)                 \
    parse1(parse1, func, _1)                                                                                          \
        __CONFIGOR_PASTE49(parse1, func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                           _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36,  \
                           _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)

#define __CONFIGOR_TO_CONF_REQUIRED(field, name) c[name].operator=(v.field);
#define __CONFIGOR_FROM_CONF_REQUIRED(field, name) c.at(name).get(v.field);

#define __CONFIGOR_TO_CONF_OPTIONAL(field, name) \
    if (v.field != decltype(v.field){})          \
    {                                            \
        __CONFIGOR_TO_CONF_REQUIRED(field, name) \
    }
#define __CONFIGOR_FROM_CONF_OPTIONAL(field, name) \
    if (c.count(name))                             \
    {                                              \
        __CONFIGOR_FROM_CONF_REQUIRED(field, name) \
    }

// REQUIRED/OPTIONAL
#define __CONFIGOR_TO_CONF_REQUIRED1(field) __CONFIGOR_TO_CONF_REQUIRED(field, #field)
#define __CONFIGOR_TO_CONF_REQUIRED2(field, name) __CONFIGOR_TO_CONF_REQUIRED(field, name)
#define __CONFIGOR_TO_CONF_OPTIONAL1(field) __CONFIGOR_TO_CONF_OPTIONAL(field, #field)
#define __CONFIGOR_TO_CONF_OPTIONAL2(field, name) __CONFIGOR_TO_CONF_OPTIONAL(field, name)
#define __CONFIGOR_FROM_CONF_REQUIRED1(field) __CONFIGOR_FROM_CONF_REQUIRED(field, #field)
#define __CONFIGOR_FROM_CONF_REQUIRED2(field, name) __CONFIGOR_FROM_CONF_REQUIRED(field, name)
#define __CONFIGOR_FROM_CONF_OPTIONAL1(field) __CONFIGOR_FROM_CONF_OPTIONAL(field, #field)
#define __CONFIGOR_FROM_CONF_OPTIONAL2(field, name) __CONFIGOR_FROM_CONF_OPTIONAL(field, name)

#define __CONFIGOR_TO_CONF_CALL_OVERLOADREQUIRED(...) \
    __CONFIGOR_EXPAND(__CONFIGOR_CALL_OVERLOAD(__CONFIGOR_TO_CONF_REQUIRED, __VA_ARGS__))
#define __CONFIGOR_TO_CONF_CALL_OVERLOADOPTIONAL(...) \
    __CONFIGOR_EXPAND(__CONFIGOR_CALL_OVERLOAD(__CONFIGOR_TO_CONF_OPTIONAL, __VA_ARGS__))
#define __CONFIGOR_FROM_CONF_CALL_OVERLOADREQUIRED(...) \
    __CONFIGOR_EXPAND(__CONFIGOR_CALL_OVERLOAD(__CONFIGOR_FROM_CONF_REQUIRED, __VA_ARGS__))
#define __CONFIGOR_FROM_CONF_CALL_OVERLOADOPTIONAL(...) \
    __CONFIGOR_EXPAND(__CONFIGOR_CALL_OVERLOAD(__CONFIGOR_FROM_CONF_OPTIONAL, __VA_ARGS__))

// Bind custom type to configor value
// e.g.
// CONFIGOR_BIND(json, myclass, REQUIRED(field1), REQUIRED(field2, "field2 name"))
// CONFIGOR_BIND(json, myclass, OPTIONAL(field1), OPTIONAL(field2, "field2 name"))
#define CONFIGOR_BIND(value_type, custom_type, ...)                                                                   \
    friend void to_value(value_type& c, const custom_type& v)                                                         \
    {                                                                                                                 \
        __CONFIGOR_EXPAND(__CONFIGOR_PASTE(__CONFIGOR_COMBINE_PASTE1, __CONFIGOR_TO_CONF_CALL_OVERLOAD, __VA_ARGS__)) \
    }                                                                                                                 \
    friend void from_value(const value_type& c, custom_type& v)                                                       \
    {                                                                                                                 \
        __CONFIGOR_EXPAND(                                                                                            \
            __CONFIGOR_PASTE(__CONFIGOR_COMBINE_PASTE1, __CONFIGOR_FROM_CONF_CALL_OVERLOAD, __VA_ARGS__))             \
    }
