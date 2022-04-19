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

template <typename Config, typename T, typename _Void = void>
struct has_to_config : std::false_type
{
};

template <typename Config, typename T>
struct has_to_config<Config, T, typename std::enable_if<!is_config<T>::value>::type>
{
private:
    using binder_type = typename Config::template binder_type<T>;

    template <typename _UTy, typename... _Args>
    using to_config_fn = decltype(_UTy::to_config(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<void, to_config_fn, binder_type, Config&, T>::value;
};

template <typename Config, typename T, typename _Void = void>
struct has_from_config : std::false_type
{
};

template <typename Config, typename T>
struct has_from_config<Config, T, typename std::enable_if<!is_config<T>::value>::type>
{
private:
    using binder_type = typename Config::template binder_type<T>;

    template <typename _UTy, typename... _Args>
    using from_config_fn = decltype(_UTy::from_config(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<void, from_config_fn, binder_type, Config, T&>::value;
};

template <typename Config, typename T, typename _Void = void>
struct has_non_default_from_config : std::false_type
{
};

template <typename Config, typename T>
struct has_non_default_from_config<Config, T, typename std::enable_if<!is_config<T>::value>::type>
{
private:
    using binder_type = typename Config::template binder_type<T>;

    template <typename _UTy, typename... _Args>
    using from_config_fn = decltype(_UTy::from_config(std::declval<_Args>()...));

public:
    static constexpr bool value = exact_detect<T, from_config_fn, binder_type, Config>::value;
};

template <typename Config, typename T, typename _Void = void>
struct is_configor_getable : std::false_type
{
};

template <typename Config, typename T>
struct is_configor_getable<Config, T, typename std::enable_if<!is_config<T>::value>::type>
{
private:
    template <typename _UTy, typename... _Args>
    using get_fn = decltype(std::declval<_UTy>().template get<_Args...>());

public:
    static constexpr bool value = exact_detect<T, get_fn, Config, T>::value;
};

// to_config functions

template <typename Config, typename T,
          typename std::enable_if<std::is_same<T, typename Config::boolean_type>::value, int>::type = 0>
void to_config(Config& c, T v)
{
    auto& cv        = c.raw_value();
    cv.type         = config_value_type::boolean;
    cv.data.boolean = v;
}

template <typename Config, typename T,
          typename std::enable_if<
              std::is_integral<T>::value && !std::is_same<T, typename Config::boolean_type>::value, int>::type = 0>
void to_config(Config& c, T v)
{
    auto& cv               = c.raw_value();
    cv.type                = config_value_type::number_integer;
    cv.data.number_integer = static_cast<typename Config::integer_type>(v);
}

template <typename Config, typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void to_config(Config& c, T v)
{
    auto& cv             = c.raw_value();
    cv.type              = config_value_type::number_float;
    cv.data.number_float = static_cast<typename Config::float_type>(v);
}

template <typename Config>
void to_config(Config& c, const typename Config::string_type& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::string;
    cv.data.string = cv.template create<typename Config::string_type>(v);
}

template <typename Config>
void to_config(Config& c, typename Config::string_type&& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::string;
    cv.data.string = cv.template create<typename Config::string_type>(std::move(v));
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<typename Config::string_type, T>::value
                                      && !std::is_same<T, typename Config::string_type>::value,
                                  int>::type = 0>
void to_config(Config& c, const T& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::string;
    cv.data.string = cv.template create<typename Config::string_type>(v);
}

template <typename Config, typename T,
          typename std::enable_if<std::is_same<T, typename Config::array_type>::value, int>::type = 0>
void to_config(Config& c, T& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::array;
    cv.data.vector = cv.template create<typename Config::array_type>(v);
}

template <typename Config, typename T,
          typename std::enable_if<std::is_same<T, typename Config::object_type>::value, int>::type = 0>
void to_config(Config& c, T& v)
{
    auto& cv       = c.raw_value();
    cv.type        = config_value_type::object;
    cv.data.object = cv.template create<typename Config::object_type>(v);
}

// from_config functions

template <typename Config, typename T,
          typename std::enable_if<std::is_same<T, typename Config::boolean_type>::value, int>::type = 0>
void from_config(const Config& c, T& v)
{
    if (!c.is_bool())
        throw make_conversion_error(c.type(), config_value_type::boolean);
    v = c.raw_value().data.boolean;
}

template <typename Config, typename T,
          typename std::enable_if<
              std::is_integral<T>::value && !std::is_same<T, typename Config::boolean_type>::value, int>::type = 0>
void from_config(const Config& c, T& v)
{
    if (!c.is_integer())
        throw make_conversion_error(c.type(), config_value_type::number_integer);
    v = static_cast<T>(c.raw_value().data.number_integer);
}

template <typename Config, typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void from_config(const Config& c, T& v)
{
    if (!c.is_float())
        throw make_conversion_error(c.type(), config_value_type::number_float);
    v = static_cast<T>(c.raw_value().data.number_float);
}

template <typename Config>
void from_config(const Config& c, typename Config::string_type& v)
{
    if (!c.is_string())
        throw make_conversion_error(c.type(), config_value_type::string);
    v = *c.raw_value().data.string;
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<T, typename Config::string_type>::value
                                      && !std::is_same<T, typename Config::string_type>::value,
                                  int>::type = 0>
void from_config(const Config& c, T& v)
{
    if (!c.is_string())
        throw make_conversion_error(c.type(), config_value_type::string);
    v = *c.raw_value().data.string;
}

template <typename Config, typename T,
          typename std::enable_if<std::is_same<T, typename Config::array_type>::value, int>::type = 0>
void from_config(const Config& c, T& v)
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

template <typename Config, typename T,
          typename std::enable_if<std::is_same<T, typename Config::object_type>::value, int>::type = 0>
void from_config(const Config& c, T& v)
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
    typename Config, typename T, size_t _Num,
    typename std::enable_if<std::is_constructible<Config, T>::value
                                && !std::is_constructible<typename Config::string_type, const T (&)[_Num]>::value,
                            int>::type = 0>
void to_config(Config& c, const T (&v)[_Num])
{
    c = nullptr;
    for (size_t i = 0; i < _Num; i++)
    {
        c[i] = v[i];
    }
}

template <
    typename Config, typename T, size_t _Num,
    typename std::enable_if<detail::is_configor_getable<Config, T>::value
                                && !std::is_constructible<typename Config::string_type, const T (&)[_Num]>::value,
                            int>::type = 0>
void from_config(const Config& c, T (&v)[_Num])
{
    for (size_t i = 0; i < c.size() && i < _Num; i++)
    {
        v[i] = c[i].template get<T>();
    }
}

// other conversions

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::unique_ptr<T>& v)
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

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::unique_ptr<T>& v)
{
    if (c.is_null())
    {
        v = nullptr;
    }
    else
    {
        v.reset(new T(c.template get<T>()));
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::shared_ptr<T>& v)
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

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::shared_ptr<T>& v)
{
    if (c.is_null())
    {
        v = nullptr;
    }
    else
    {
        v = std::make_shared<T>(c.template get<T>());
    }
}

template <typename Config, typename T, size_t _Num,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::array<T, _Num>& v)
{
    c = nullptr;
    for (size_t i = 0; i < _Num; i++)
    {
        c[i] = v.at(i);
    }
}

template <typename Config, typename T, size_t _Num,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::array<T, _Num>& v)
{
    for (size_t i = 0; i < c.size() && i < _Num; i++)
    {
        v[i] = c[i].template get<T>();
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::vector<T>& v)
{
    c = nullptr;
    for (size_t i = 0; i < v.size(); ++i)
    {
        c[i] = v.at(i);
    }
}

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::vector<T>& v)
{
    v.resize(c.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        v[i] = c[i].template get<T>();
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::deque<T>& v)
{
    c = nullptr;
    for (size_t i = 0; i < v.size(); ++i)
    {
        c[i] = v.at(i);
    }
}

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::deque<T>& v)
{
    v.resize(c.size());
    for (size_t i = 0; i < c.size(); ++i)
    {
        v[i] = c[i].template get<T>();
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::list<T>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; iter != v.end(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::list<T>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.push_back(c[i].template get<T>());
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::forward_list<T>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; iter != v.end(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::forward_list<T>& v)
{
    v.clear();
    size_t size = c.size();
    for (size_t i = 0; i < size; i++)
    {
        v.push_front(c[size - i - 1].template get<T>());
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::set<T>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; i < v.size(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::set<T>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.insert(c[i].template get<T>());
    }
}

template <typename Config, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value, int>::type = 0>
void to_config(Config& c, const std::unordered_set<T>& v)
{
    c         = nullptr;
    auto iter = v.begin();
    for (size_t i = 0; i < v.size(); ++i, ++iter)
    {
        c[i] = *iter;
    }
}

template <typename Config, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value, int>::type = 0>
void from_config(const Config& c, std::unordered_set<T>& v)
{
    v.clear();
    for (size_t i = 0; i < c.size(); i++)
    {
        v.insert(c[i].template get<T>());
    }
}

template <typename Config, typename _KeyTy, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value
                                      && std::is_constructible<typename Config::string_type, _KeyTy>::value,
                                  int>::type = 0>
void to_config(Config& c, const std::map<_KeyTy, T>& v)
{
    c = nullptr;
    for (const auto& p : v)
    {
        c[p.first] = p.second;
    }
}

template <typename Config, typename _KeyTy, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value
                                      && std::is_constructible<_KeyTy, typename Config::string_type>::value,
                                  int>::type = 0>
void from_config(const Config& c, std::map<_KeyTy, T>& v)
{
    for (auto iter = c.cbegin(); iter != c.cend(); iter++)
    {
        v.emplace(iter.key(), iter.value().template get<T>());
    }
}

template <typename Config, typename _KeyTy, typename T,
          typename std::enable_if<std::is_constructible<Config, T>::value
                                      && std::is_constructible<typename Config::string_type, _KeyTy>::value,
                                  int>::type = 0>
void to_config(Config& c, const std::unordered_map<_KeyTy, T>& v)
{
    c = nullptr;
    for (const auto& p : v)
    {
        c[p.first] = p.second;
    }
}

template <typename Config, typename _KeyTy, typename T,
          typename std::enable_if<detail::is_configor_getable<Config, T>::value
                                      && std::is_constructible<_KeyTy, typename Config::string_type>::value,
                                  int>::type = 0>
void from_config(const Config& c, std::unordered_map<_KeyTy, T>& v)
{
    for (auto iter = c.cbegin(); iter != c.cend(); iter++)
    {
        v.emplace(iter.key(), iter.value().template get<T>());
    }
}

//
// to_config & from_config function object
// Explanation: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html
//
struct to_config_fn
{
    template <typename Config, typename T>
    auto operator()(Config& c, T&& v) const noexcept(noexcept(to_config(c, std::forward<T>(v))))
        -> decltype(to_config(c, std::forward<T>(v)))
    {
        return to_config(c, std::forward<T>(v));
    }
};

struct from_config_fn
{
    template <typename Config, typename T>
    auto operator()(const Config& c, T&& v) const noexcept(noexcept(from_config(c, std::forward<T>(v))))
        -> decltype(from_config(c, std::forward<T>(v)))
    {
        return from_config(c, std::forward<T>(v));
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

template <typename T>
class config_binder
{
public:
    template <typename Config, typename _UTy = T>
    static auto to_config(Config& c, _UTy&& v) noexcept(noexcept(::configor::to_config(c, std::forward<_UTy>(v))))
        -> decltype(::configor::to_config(c, std::forward<_UTy>(v)))
    {
        return ::configor::to_config(c, std::forward<_UTy>(v));
    }

    template <typename Config, typename _UTy = T>
    static auto from_config(Config&& c,
                            _UTy&     v) noexcept(noexcept(::configor::from_config(std::forward<Config>(c), v)))
        -> decltype(::configor::from_config(std::forward<Config>(c), v))
    {
        return ::configor::from_config(std::forward<Config>(c), v);
    }
};

namespace detail
{
//
// configor_wrapper
//

template <typename Config, typename T>
class read_configor_wrapper
{
public:
    using char_type = typename Config::char_type;

    explicit read_configor_wrapper(const T& v)
        : v_(v)
    {
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& out,
                                                     const read_configor_wrapper&   wrapper)
    {
        out << Config(wrapper.v_);
        return out;
    }

private:
    const T& v_;
};

template <typename Config, typename T>
class write_configor_wrapper : public read_configor_wrapper<Config, T>
{
public:
    using char_type = typename Config::char_type;

    explicit write_configor_wrapper(T& v)
        : read_configor_wrapper<Config, T>(v)
        , v_(v)
    {
    }

    friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& in,
                                                     const write_configor_wrapper&  wrapper)
    {
        Config c{};
        in >> c;
        const_cast<T&>(wrapper.v_) = c.template get<T>();
        return in;
    }

private:
    T& v_;
};
}  // namespace detail

}  // namespace configor

#define CONFIGOR_EXPAND(x) x
#define CONFIGOR_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                           NAME, ...)                                                                                 \
    NAME
#define CONFIGOR_PASTE(...)                                                                                         \
    CONFIGOR_EXPAND(CONFIGOR_GET_MACRO(                                                                             \
        __VA_ARGS__, CONFIGOR_PASTE20, CONFIGOR_PASTE19, CONFIGOR_PASTE18, CONFIGOR_PASTE17, CONFIGOR_PASTE16,      \
        CONFIGOR_PASTE15, CONFIGOR_PASTE14, CONFIGOR_PASTE13, CONFIGOR_PASTE12, CONFIGOR_PASTE11, CONFIGOR_PASTE10, \
        CONFIGOR_PASTE9, CONFIGOR_PASTE8, CONFIGOR_PASTE7, CONFIGOR_PASTE6, CONFIGOR_PASTE5, CONFIGOR_PASTE4,       \
        CONFIGOR_PASTE3, CONFIGOR_PASTE2, CONFIGOR_PASTE1)(__VA_ARGS__))
#define CONFIGOR_PASTE2(func, v1) func(v1)
#define CONFIGOR_PASTE3(func, v1, v2) CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE2(func, v2)
#define CONFIGOR_PASTE4(func, v1, v2, v3) CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE3(func, v2, v3)
#define CONFIGOR_PASTE5(func, v1, v2, v3, v4) CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE4(func, v2, v3, v4)
#define CONFIGOR_PASTE6(func, v1, v2, v3, v4, v5) CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE5(func, v2, v3, v4, v5)
#define CONFIGOR_PASTE7(func, v1, v2, v3, v4, v5, v6) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE6(func, v2, v3, v4, v5, v6)
#define CONFIGOR_PASTE8(func, v1, v2, v3, v4, v5, v6, v7) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE7(func, v2, v3, v4, v5, v6, v7)
#define CONFIGOR_PASTE9(func, v1, v2, v3, v4, v5, v6, v7, v8) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE8(func, v2, v3, v4, v5, v6, v7, v8)
#define CONFIGOR_PASTE10(func, v1, v2, v3, v4, v5, v6, v7, v8, v9) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE9(func, v2, v3, v4, v5, v6, v7, v8, v9)
#define CONFIGOR_PASTE11(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE10(func, v2, v3, v4, v5, v6, v7, v8, v9, v10)
#define CONFIGOR_PASTE12(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE11(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11)
#define CONFIGOR_PASTE13(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE12(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12)
#define CONFIGOR_PASTE14(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE13(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13)
#define CONFIGOR_PASTE15(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE14(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14)
#define CONFIGOR_PASTE16(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE15(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15)
#define CONFIGOR_PASTE17(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
    CONFIGOR_PASTE2(func, v1) CONFIGOR_PASTE16(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16)
#define CONFIGOR_PASTE18(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17) \
    CONFIGOR_PASTE2(func, v1)                                                                              \
    CONFIGOR_PASTE17(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17)
#define CONFIGOR_PASTE19(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18) \
    CONFIGOR_PASTE2(func, v1)                                                                                   \
    CONFIGOR_PASTE18(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18)
#define CONFIGOR_PASTE20(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19) \
    CONFIGOR_PASTE2(func, v1)                                                                                        \
    CONFIGOR_PASTE19(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19)

#define CONFIGOR_TO_CONF(field) c[#field].operator=(v.field);
#define CONFIGOR_FROM_CONF(field) c.at(#field).get(v.field);

#define CONFIGOR_BIND_WITH_CONF(config_type, value_type, ...)            \
    friend void to_config(config_type& c, const value_type& v)           \
    {                                                                    \
        CONFIGOR_EXPAND(CONFIGOR_PASTE(CONFIGOR_TO_CONF, __VA_ARGS__))   \
    }                                                                    \
    friend void from_config(const config_type& c, value_type& v)         \
    {                                                                    \
        CONFIGOR_EXPAND(CONFIGOR_PASTE(CONFIGOR_FROM_CONF, __VA_ARGS__)) \
    }

#define CONFIGOR_BIND(value_type, ...) CONFIGOR_BIND_WITH_CONF(basic_config<>, value_type, __VA_ARGS__)
