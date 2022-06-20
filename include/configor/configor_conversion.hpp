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
#include "configor_declare.hpp"
#include "configor_value.hpp"

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

//
// type_traits
//

namespace detail
{

template <typename _ConfTy, typename _Ty, typename _Void = void>
struct has_to_config : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct has_to_config<_ConfTy, _Ty, typename std::enable_if<!is_config<_Ty>::value>::type>
{
private:
    using binder_type = typename _ConfTy::template binder_type<_Ty>;

    template <typename _UTy, typename... _Args>
    using to_config_fn = decltype(_UTy::to_config(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<void, to_config_fn, binder_type, _ConfTy&, _Ty>::value;
};

template <typename _ConfTy, typename _Ty, typename _Void = void>
struct has_from_config : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct has_from_config<_ConfTy, _Ty, typename std::enable_if<!is_config<_Ty>::value>::type>
{
private:
    using binder_type = typename _ConfTy::template binder_type<_Ty>;

    template <typename _UTy, typename... _Args>
    using from_config_fn = decltype(_UTy::from_config(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<void, from_config_fn, binder_type, _ConfTy, _Ty&>::value;
};

template <typename _ConfTy, typename _Ty, typename _Void = void>
struct has_non_default_from_config : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct has_non_default_from_config<_ConfTy, _Ty, typename std::enable_if<!is_config<_Ty>::value>::type>
{
private:
    using binder_type = typename _ConfTy::template binder_type<_Ty>;

    template <typename _UTy, typename... _Args>
    using from_config_fn = decltype(_UTy::from_config(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<_Ty, from_config_fn, binder_type, _ConfTy>::value;
};

template <typename _ConfTy, typename _Ty, typename _Void = void>
struct is_configor_getable : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct is_configor_getable<_ConfTy, _Ty, typename std::enable_if<!is_config<_Ty>::value>::type>
{
private:
    template <typename _UTy, typename... _Args>
    using get_fn = decltype(std::declval<_UTy>().template get<_Args...>());

public:
    static constexpr bool value = exact_detect<_Ty, get_fn, _ConfTy, _Ty>::value;
};

// to_config functions

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ConfTy::boolean_type>::value, int>::type = 0>
void to_config(_ConfTy& c, _Ty v)
{
    auto& cv        = c.raw_value();
    cv.type         = config_value_type::boolean;
    cv.data.boolean = v;
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<
              std::is_integral<_Ty>::value && !std::is_same<_Ty, typename _ConfTy::boolean_type>::value, int>::type = 0>
void to_config(_ConfTy& c, _Ty v)
{
    auto& cv               = c.raw_value();
    cv.type                = config_value_type::number_integer;
    cv.data.number_integer = static_cast<typename _ConfTy::integer_type>(v);
}

template <typename _ConfTy, typename _Ty, typename std::enable_if<std::is_floating_point<_Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, _Ty v)
{
    auto& cv             = c.raw_value();
    cv.type              = config_value_type::number_float;
    cv.data.number_float = static_cast<typename _ConfTy::float_type>(v);
}

template <typename _ConfTy>
void to_config(_ConfTy& c, const typename _ConfTy::string_type& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::string;
    cv.data.string = cv.template create<typename _ConfTy::string_type>(v);
}

template <typename _ConfTy>
void to_config(_ConfTy& c, typename _ConfTy::string_type&& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::string;
    cv.data.string = cv.template create<typename _ConfTy::string_type>(std::move(v));
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<typename _ConfTy::string_type, _Ty>::value
                                      && !std::is_same<_Ty, typename _ConfTy::string_type>::value,
                                  int>::type = 0>
void to_config(_ConfTy& c, const _Ty& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::string;
    cv.data.string = cv.template create<typename _ConfTy::string_type>(v);
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ConfTy::array_type>::value, int>::type = 0>
void to_config(_ConfTy& c, _Ty& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::array;
    cv.data.vector = cv.template create<typename _ConfTy::array_type>(v);
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ConfTy::object_type>::value, int>::type = 0>
void to_config(_ConfTy& c, _Ty& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::object;
    cv.data.object = cv.template create<typename _ConfTy::object_type>(v);
}

// from_config functions

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ConfTy::boolean_type>::value, int>::type = 0>
void from_config(const _ConfTy& c, _Ty& v)
{
    if (!c.is_bool())
        throw make_conversion_error(c.type(), config_value_type::boolean);
    v = c.raw_value().data.boolean;
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<
              std::is_integral<_Ty>::value && !std::is_same<_Ty, typename _ConfTy::boolean_type>::value, int>::type = 0>
void from_config(const _ConfTy& c, _Ty& v)
{
    if (!c.is_integer())
        throw make_conversion_error(c.type(), config_value_type::number_integer);
    v = static_cast<_Ty>(c.raw_value().data.number_integer);
}

template <typename _ConfTy, typename _Ty, typename std::enable_if<std::is_floating_point<_Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, _Ty& v)
{
    if (!c.is_float())
        throw make_conversion_error(c.type(), config_value_type::number_float);
    v = static_cast<_Ty>(c.raw_value().data.number_float);
}

template <typename _ConfTy>
void from_config(const _ConfTy& c, typename _ConfTy::string_type& v)
{
    if (!c.is_string())
        throw make_conversion_error(c.type(), config_value_type::string);
    v = *c.raw_value().data.string;
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_Ty, typename _ConfTy::string_type>::value
                                      && !std::is_same<_Ty, typename _ConfTy::string_type>::value,
                                  int>::type = 0>
void from_config(const _ConfTy& c, _Ty& v)
{
    if (!c.is_string())
        throw make_conversion_error(c.type(), config_value_type::string);
    v = *c.raw_value().data.string;
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ConfTy::array_type>::value, int>::type = 0>
void from_config(const _ConfTy& c, _Ty& v)
{
    if (c.is_null())
    {
        v.clear();
        return;
    }
    if (!c.is_array())
        throw make_conversion_error(c.type(), config_value_type::array);
    v.assign((*c.raw_value().data.vector).begin(), (*c.raw_value().data.vector).end());
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_same<_Ty, typename _ConfTy::object_type>::value, int>::type = 0>
void from_config(const _ConfTy& c, _Ty& v)
{
    if (c.is_null())
    {
        v.clear();
        return;
    }
    if (!c.is_object())
        throw make_conversion_error(c.type(), config_value_type::object);
    v = *c.raw_value().data.object;
}

// c-style array

template <
    typename _ConfTy, typename _Ty, size_t _Num,
    typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value
                                && !std::is_constructible<typename _ConfTy::string_type, const _Ty (&)[_Num]>::value,
                            int>::type = 0>
void to_config(_ConfTy& c, const _Ty (&v)[_Num])
{
    c = nullptr;
    for (size_t i = 0; i < _Num; i++)
    {
        c[i] = v[i];
    }
}

template <
    typename _ConfTy, typename _Ty, size_t _Num,
    typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value
                                && !std::is_constructible<typename _ConfTy::string_type, const _Ty (&)[_Num]>::value,
                            int>::type = 0>
void from_config(const _ConfTy& c, _Ty (&v)[_Num])
{
    for (size_t i = 0; i < c.size() && i < _Num; i++)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

// other conversions

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::unique_ptr<_Ty>& v)
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

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::unique_ptr<_Ty>& v)
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

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::shared_ptr<_Ty>& v)
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

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::shared_ptr<_Ty>& v)
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

template <typename _ConfTy, typename _Ty, size_t _Num,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::array<_Ty, _Num>& v)
{
    c = nullptr;
    for (size_t i = 0; i < _Num; i++)
    {
        c[i] = v.at(i);
    }
}

template <typename _ConfTy, typename _Ty, size_t _Num,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::array<_Ty, _Num>& v)
{
    for (size_t i = 0; i < c.size() && i < _Num; i++)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::vector<_Ty>& v)
{
    c = nullptr;
    for (size_t i = 0; i < v.size(); ++i)
    {
        c[i] = v.at(i);
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::vector<_Ty>& v)
{
    v.resize(c.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::deque<_Ty>& v)
{
    c = nullptr;
    for (size_t i = 0; i < v.size(); ++i)
    {
        c[i] = v.at(i);
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::deque<_Ty>& v)
{
    v.resize(c.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        v[i] = c[i].template get<_Ty>();
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::list<_Ty>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; iter != v.end(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::list<_Ty>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.push_back(c[i].template get<_Ty>());
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::forward_list<_Ty>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; iter != v.end(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::forward_list<_Ty>& v)
{
    v.clear();
    size_t size = c.size();
    for (size_t i = 0; i < size; i++)
    {
        v.push_front(c[size - i - 1].template get<_Ty>());
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::set<_Ty>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; i < v.size(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::set<_Ty>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.insert(c[i].template get<_Ty>());
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value, int>::type = 0>
void to_config(_ConfTy& c, const std::unordered_set<_Ty>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; i < v.size(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename _ConfTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value, int>::type = 0>
void from_config(const _ConfTy& c, std::unordered_set<_Ty>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.insert(c[i].template get<_Ty>());
    }
}

template <typename _ConfTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value
                                      && std::is_constructible<typename _ConfTy::string_type, _KeyTy>::value,
                                  int>::type = 0>
void to_config(_ConfTy& c, const std::map<_KeyTy, _Ty>& v)
{
    c = nullptr;
    for (const auto& p : v)
    {
        c[p.first] = p.second;
    }
}

template <typename _ConfTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value
                                      && std::is_constructible<_KeyTy, typename _ConfTy::string_type>::value,
                                  int>::type = 0>
void from_config(const _ConfTy& c, std::map<_KeyTy, _Ty>& v)
{
    for (auto iter = c.cbegin(); iter != c.cend(); iter++)
    {
        v.emplace(iter.key(), iter.value().template get<_Ty>());
    }
}

template <typename _ConfTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<std::is_constructible<_ConfTy, _Ty>::value
                                      && std::is_constructible<typename _ConfTy::string_type, _KeyTy>::value,
                                  int>::type = 0>
void to_config(_ConfTy& c, const std::unordered_map<_KeyTy, _Ty>& v)
{
    c = nullptr;
    for (const auto& p : v)
    {
        c[p.first] = p.second;
    }
}

template <typename _ConfTy, typename _KeyTy, typename _Ty,
          typename std::enable_if<detail::is_configor_getable<_ConfTy, _Ty>::value
                                      && std::is_constructible<_KeyTy, typename _ConfTy::string_type>::value,
                                  int>::type = 0>
void from_config(const _ConfTy& c, std::unordered_map<_KeyTy, _Ty>& v)
{
    for (auto iter = c.cbegin(); iter != c.cend(); iter++)
    {
        v.emplace(iter.key(), iter.value().template get<_Ty>());
    }
}

//
// to_config & from_config function object
// Explanation: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html
//
struct to_config_fn
{
    template <typename _ConfTy, typename _Ty>
    auto operator()(_ConfTy& c, _Ty&& v) const noexcept(noexcept(to_config(c, std::forward<_Ty>(v))))
        -> decltype(to_config(c, std::forward<_Ty>(v)))
    {
        return to_config(c, std::forward<_Ty>(v));
    }
};

struct from_config_fn
{
    template <typename _ConfTy, typename _Ty>
    auto operator()(const _ConfTy& c, _Ty&& v) const noexcept(noexcept(from_config(c, std::forward<_Ty>(v))))
        -> decltype(from_config(c, std::forward<_Ty>(v)))
    {
        return from_config(c, std::forward<_Ty>(v));
    }
};

}  // namespace detail

namespace
{
constexpr auto const& to_config   = detail::static_const<detail::to_config_fn>::value;
constexpr auto const& from_config = detail::static_const<detail::from_config_fn>::value;
}  // namespace

//
// config_binder
//

template <typename _Ty>
class config_binder
{
public:
    template <typename _ConfTy, typename _UTy = _Ty>
    static auto to_config(_ConfTy& c, _UTy&& v) noexcept(noexcept(::configor::to_config(c, std::forward<_UTy>(v))))
        -> decltype(::configor::to_config(c, std::forward<_UTy>(v)))
    {
        return ::configor::to_config(c, std::forward<_UTy>(v));
    }

    template <typename _ConfTy, typename _UTy = _Ty>
    static auto from_config(_ConfTy&& c,
                            _UTy&     v) noexcept(noexcept(::configor::from_config(std::forward<_ConfTy>(c), v)))
        -> decltype(::configor::from_config(std::forward<_ConfTy>(c), v))
    {
        return ::configor::from_config(std::forward<_ConfTy>(c), v);
    }
};

namespace detail
{
//
// configor_wrapper
//

template <typename _ConfTy, typename _Ty>
class read_configor_wrapper
{
public:
    using char_type = typename _ConfTy::char_type;

    explicit read_configor_wrapper(const _Ty& v)
        : v_(v)
    {
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& out,
                                                     const read_configor_wrapper&   wrapper)
    {
        out << _ConfTy(wrapper.v_);
        return out;
    }

private:
    const _Ty& v_;
};

template <typename _ConfTy, typename _Ty>
class write_configor_wrapper : public read_configor_wrapper<_ConfTy, _Ty>
{
public:
    using char_type = typename _ConfTy::char_type;

    explicit write_configor_wrapper(_Ty& v)
        : read_configor_wrapper<_ConfTy, _Ty>(v)
        , v_(v)
    {
    }

    friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& in,
                                                     const write_configor_wrapper&  wrapper)
    {
        _ConfTy c{};
        in >> c;
        const_cast<_Ty&>(wrapper.v_) = c.template get<_Ty>();
        return in;
    }

private:
    _Ty& v_;
};
}  // namespace detail

}  // namespace configor

#define __CONFIGOR_COMBINE_INNER(a, b) a##b
#define __CONFIGOR_COMBINE(a, b) __CONFIGOR_COMBINE_INNER(a, b)

// __CONFIGOR_EXPAND is a solution for this question:
// https://stackoverflow.com/questions/9183993/msvc-variadic-macro-expansion
#define __CONFIGOR_EXPAND(x) x
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

#define __CONFIGOR_PASTE(...)                                                                               \
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
        __CONFIGOR_PASTE1)(__VA_ARGS__))

#define __CONFIGOR_PASTE2(func, _1) func##_1
#define __CONFIGOR_PASTE3(func, _1, _2) __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE2(func, _2)
#define __CONFIGOR_PASTE4(func, _1, _2, _3) __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE3(func, _2, _3)
#define __CONFIGOR_PASTE5(func, _1, _2, _3, _4) __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE4(func, _2, _3, _4)
#define __CONFIGOR_PASTE6(func, _1, _2, _3, _4, _5) __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE5(func, _2, _3, _4, _5)
#define __CONFIGOR_PASTE7(func, _1, _2, _3, _4, _5, _6) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE6(func, _2, _3, _4, _5, _6)
#define __CONFIGOR_PASTE8(func, _1, _2, _3, _4, _5, _6, _7) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE7(func, _2, _3, _4, _5, _6, _7)
#define __CONFIGOR_PASTE9(func, _1, _2, _3, _4, _5, _6, _7, _8) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE8(func, _2, _3, _4, _5, _6, _7, _8)
#define __CONFIGOR_PASTE10(func, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE9(func, _2, _3, _4, _5, _6, _7, _8, _9)
#define __CONFIGOR_PASTE11(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE10(func, _2, _3, _4, _5, _6, _7, _8, _9, _10)
#define __CONFIGOR_PASTE12(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE11(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)
#define __CONFIGOR_PASTE13(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE12(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)
#define __CONFIGOR_PASTE14(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE13(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)
#define __CONFIGOR_PASTE15(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE14(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)
#define __CONFIGOR_PASTE16(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
    __CONFIGOR_PASTE2(func, _1) __CONFIGOR_PASTE15(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)
#define __CONFIGOR_PASTE17(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
    __CONFIGOR_PASTE2(func, _1)                                                                         \
    __CONFIGOR_PASTE16(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)
#define __CONFIGOR_PASTE18(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
    __CONFIGOR_PASTE2(func, _1)                                                                              \
    __CONFIGOR_PASTE17(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17)
#define __CONFIGOR_PASTE19(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) \
    __CONFIGOR_PASTE2(func, _1)                                                                                   \
    __CONFIGOR_PASTE18(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18)
#define __CONFIGOR_PASTE20(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19) \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE19(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19)
#define __CONFIGOR_PASTE21(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20)                                                                                        \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE20(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20)
#define __CONFIGOR_PASTE22(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21)                                                                                   \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE21(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21)
#define __CONFIGOR_PASTE23(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22)                                                                              \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE22(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22)
#define __CONFIGOR_PASTE24(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23)                                                                         \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE23(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23)
#define __CONFIGOR_PASTE25(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24)                                                                    \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE24(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24)
#define __CONFIGOR_PASTE26(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25)                                                               \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE25(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25)
#define __CONFIGOR_PASTE27(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26)                                                          \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE26(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26)
#define __CONFIGOR_PASTE28(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27)                                                     \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE27(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27)
#define __CONFIGOR_PASTE29(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28)                                                \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE28(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28)
#define __CONFIGOR_PASTE30(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29)                                           \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE29(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29)
#define __CONFIGOR_PASTE31(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)                                      \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE30(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)
#define __CONFIGOR_PASTE32(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)                                 \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE31(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)
#define __CONFIGOR_PASTE33(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)                            \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE32(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)
#define __CONFIGOR_PASTE34(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)                       \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE33(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)
#define __CONFIGOR_PASTE35(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)                  \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE34(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)
#define __CONFIGOR_PASTE36(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)             \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE35(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)
#define __CONFIGOR_PASTE37(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36)        \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE36(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36)
#define __CONFIGOR_PASTE38(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37)   \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE37(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37)
#define __CONFIGOR_PASTE39(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38)                                                                                        \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE38(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38)
#define __CONFIGOR_PASTE40(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39)                                                                                   \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE39(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39)
#define __CONFIGOR_PASTE41(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40)                                                                              \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE40(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40)
#define __CONFIGOR_PASTE42(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41)                                                                         \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE41(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41)
#define __CONFIGOR_PASTE43(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42)                                                                    \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE42(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42)
#define __CONFIGOR_PASTE44(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43)                                                               \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE43(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43)
#define __CONFIGOR_PASTE45(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43, _44)                                                          \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE44(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43, _44)
#define __CONFIGOR_PASTE46(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43, _44, _45)                                                     \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE45(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43, _44, _45)
#define __CONFIGOR_PASTE47(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43, _44, _45, _46)                                                \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE46(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43, _44, _45, _46)
#define __CONFIGOR_PASTE48(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)                                           \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE47(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43, _44, _45, _46, _47)
#define __CONFIGOR_PASTE49(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)                                      \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE48(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43, _44, _45, _46, _47, _48)
#define __CONFIGOR_PASTE50(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
                           _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                           _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)                                 \
    __CONFIGOR_PASTE2(func, _1)                                                                                        \
    __CONFIGOR_PASTE49(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,    \
                       _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,  \
                       _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)

#define __CONFIGOR_TO_CONF_REQUIRED(field) c[#field].operator=(v.field);
#define __CONFIGOR_FROM_CONF_REQUIRED(field) c.at(#field).get(v.field);

#define __CONFIGOR_TO_CONF_OPTIONAL(field) \
    __CONFIGOR_EXPAND(if (v.field != decltype(v.field){}) __CONFIGOR_TO_CONF_REQUIRED(field))
#define __CONFIGOR_FROM_CONF_OPTIONAL(field) \
    __CONFIGOR_EXPAND(if (c.count(#field)) __CONFIGOR_FROM_CONF_REQUIRED(field))

#define CONFIGOR_REQUIRED(field) REQUIRED(field)
#define CONFIGOR_OPTIONAL(field) OPTIONAL(field)

#define CONFIGOR_BIND(config_type, value_type, ...)                             \
    friend void to_config(config_type& c, const value_type& v)                  \
    {                                                                           \
        __CONFIGOR_EXPAND(__CONFIGOR_PASTE(__CONFIGOR_TO_CONF_, __VA_ARGS__))   \
    }                                                                           \
    friend void from_config(const config_type& c, value_type& v)                \
    {                                                                           \
        __CONFIGOR_EXPAND(__CONFIGOR_PASTE(__CONFIGOR_FROM_CONF_, __VA_ARGS__)) \
    }

#define __CONFIGOR_REQUIRED_COMMA1(_1) , REQUIRED(_1)
#define __CONFIGOR_REQUIRED_COMMA2(_1, _2) __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA1(_2)
#define __CONFIGOR_REQUIRED_COMMA3(_1, _2, _3) __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA2(_2, _3)
#define __CONFIGOR_REQUIRED_COMMA4(_1, _2, _3, _4) __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA3(_2, _3, _4)
#define __CONFIGOR_REQUIRED_COMMA5(_1, _2, _3, _4, _5) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA4(_2, _3, _4, _5)
#define __CONFIGOR_REQUIRED_COMMA6(_1, _2, _3, _4, _5, _6) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA5(_2, _3, _4, _5, _6)
#define __CONFIGOR_REQUIRED_COMMA7(_1, _2, _3, _4, _5, _6, _7) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA6(_2, _3, _4, _5, _6, _7)
#define __CONFIGOR_REQUIRED_COMMA8(_1, _2, _3, _4, _5, _6, _7, _8) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA7(_2, _3, _4, _5, _6, _7, _8)
#define __CONFIGOR_REQUIRED_COMMA9(_1, _2, _3, _4, _5, _6, _7, _8, _9) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA8(_2, _3, _4, _5, _6, _7, _8, _9)
#define __CONFIGOR_REQUIRED_COMMA10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA9(_2, _3, _4, _5, _6, _7, _8, _9, _10)
#define __CONFIGOR_REQUIRED_COMMA11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA10(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11)
#define __CONFIGOR_REQUIRED_COMMA12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA11(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)
#define __CONFIGOR_REQUIRED_COMMA13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA12(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)
#define __CONFIGOR_REQUIRED_COMMA14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
    __CONFIGOR_REQUIRED_COMMA1(_1) __CONFIGOR_REQUIRED_COMMA13(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)
#define __CONFIGOR_REQUIRED_COMMA15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                    \
    __CONFIGOR_REQUIRED_COMMA14(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)
#define __CONFIGOR_REQUIRED_COMMA16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                         \
    __CONFIGOR_REQUIRED_COMMA15(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)
#define __CONFIGOR_REQUIRED_COMMA17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                              \
    __CONFIGOR_REQUIRED_COMMA16(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17)
#define __CONFIGOR_REQUIRED_COMMA18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                   \
    __CONFIGOR_REQUIRED_COMMA17(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18)
#define __CONFIGOR_REQUIRED_COMMA19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                                    _19)                                                                             \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                   \
    __CONFIGOR_REQUIRED_COMMA18(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19)
#define __CONFIGOR_REQUIRED_COMMA20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                                    _19, _20)                                                                        \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                   \
    __CONFIGOR_REQUIRED_COMMA19(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20)
#define __CONFIGOR_REQUIRED_COMMA21(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21)                                                                     \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA20(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21)
#define __CONFIGOR_REQUIRED_COMMA22(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22)                                                                \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA21(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22)
#define __CONFIGOR_REQUIRED_COMMA23(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23)                                                           \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA22(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23)
#define __CONFIGOR_REQUIRED_COMMA24(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24)                                                      \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA23(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24)
#define __CONFIGOR_REQUIRED_COMMA25(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25)                                                 \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA24(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25)
#define __CONFIGOR_REQUIRED_COMMA26(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26)                                            \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA25(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26)
#define __CONFIGOR_REQUIRED_COMMA27(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27)                                       \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA26(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27)
#define __CONFIGOR_REQUIRED_COMMA28(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28)                                  \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA27(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28)
#define __CONFIGOR_REQUIRED_COMMA29(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29)                             \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA28(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29)
#define __CONFIGOR_REQUIRED_COMMA30(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)                        \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA29(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)
#define __CONFIGOR_REQUIRED_COMMA31(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)                   \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA30(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)
#define __CONFIGOR_REQUIRED_COMMA32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)              \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA31(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)
#define __CONFIGOR_REQUIRED_COMMA33(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)         \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA32(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)
#define __CONFIGOR_REQUIRED_COMMA34(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)    \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA33(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)
#define __CONFIGOR_REQUIRED_COMMA35(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35)                                                                               \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA34(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)
#define __CONFIGOR_REQUIRED_COMMA36(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36)                                                                          \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA35(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36)
#define __CONFIGOR_REQUIRED_COMMA37(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37)                                                                     \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA36(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37)
#define __CONFIGOR_REQUIRED_COMMA38(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38)                                                                \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA37(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38)
#define __CONFIGOR_REQUIRED_COMMA39(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39)                                                           \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA38(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39)
#define __CONFIGOR_REQUIRED_COMMA40(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40)                                                      \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA39(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40)
#define __CONFIGOR_REQUIRED_COMMA41(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41)                                                 \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA40(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41)
#define __CONFIGOR_REQUIRED_COMMA42(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42)                                            \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA41(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42)
#define __CONFIGOR_REQUIRED_COMMA43(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43)                                       \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA42(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43)
#define __CONFIGOR_REQUIRED_COMMA44(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44)                                  \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA43(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44)
#define __CONFIGOR_REQUIRED_COMMA45(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45)                             \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA44(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44, _45)
#define __CONFIGOR_REQUIRED_COMMA46(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46)                        \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA45(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44, _45, _46)
#define __CONFIGOR_REQUIRED_COMMA47(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)                   \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA46(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44, _45, _46, _47)
#define __CONFIGOR_REQUIRED_COMMA48(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)              \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA47(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48)
#define __CONFIGOR_REQUIRED_COMMA49(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)         \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA48(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)
#define __CONFIGOR_REQUIRED_COMMA50(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,   \
                                    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34,    \
                                    _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)    \
    __CONFIGOR_REQUIRED_COMMA1(_1)                                                                                     \
    __CONFIGOR_REQUIRED_COMMA49(_2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37,   \
                                _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)

#define __CONFIGOR_BIND_WRAPPER(...) __CONFIGOR_EXPAND(CONFIGOR_BIND(__VA_ARGS__))

#define CONFIGOR_BIND_ALL_REQUIRED(config_type, value_type, ...) \
    __CONFIGOR_EXPAND(__CONFIGOR_BIND_WRAPPER(                   \
        config_type, value_type __CONFIGOR_CALL_OVERLOAD(__CONFIGOR_REQUIRED_COMMA, __VA_ARGS__)))
