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

template <typename _ConfTy, typename _Ty>
inline auto adl_to_config(_ConfTy& c, const _Ty& value) -> decltype(to_config(c, value))
{
    return to_config(c, value);
}

template <typename _ConfTy, typename _Ty>
inline auto adl_from_config(const _ConfTy& c, _Ty& value) -> decltype(from_config(c, value))
{
    return from_config(c, value);
}

}  // namespace detail

//
// config_bind
//

template <typename _Ty>
struct config_bind
{
    template <typename _ConfTy, typename _UTy = _Ty,
              typename = typename std::enable_if<std::is_void<decltype(::configor::detail::adl_to_config(
                  std::declval<_ConfTy&>(), std::declval<const _UTy&>()))>::value>::type>
    static inline void to_config(_ConfTy& c, const _UTy& value)
    {
        ::configor::detail::adl_to_config(c, value);
    }

    template <typename _ConfTy, typename _UTy = _Ty,
              typename = typename std::enable_if<std::is_void<decltype(::configor::detail::adl_from_config(
                  std::declval<const _ConfTy&>(), std::declval<_UTy&>()))>::value>::type>
    static inline void from_config(const _ConfTy& c, _UTy& value)
    {
        ::configor::detail::adl_from_config(c, value);
    }
};

//
// type_traits
//

namespace detail
{

template <typename _ConfTy, typename _Ty, typename _Check = void>
struct has_to_config : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct has_to_config<_ConfTy, _Ty,
                     typename std::enable_if<std::is_void<decltype(config_bind<_Ty>::to_config(
                                                 std::declval<_ConfTy&>(), std::declval<const _Ty&>()))>::value,
                                             void>::type> : std::true_type
{
};

template <typename _ConfTy, typename _Ty, typename _Check = void>
struct has_default_from_config : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct has_default_from_config<
    _ConfTy, _Ty,
    typename std::enable_if<std::is_void<decltype(config_bind<_Ty>::from_config(std::declval<const _ConfTy&>(),
                                                                                std::declval<_Ty&>()))>::value,
                            void>::type> : std::true_type
{
};

template <typename _ConfTy, typename _Ty, typename _Check = void>
struct has_non_default_from_config : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct has_non_default_from_config<
    _ConfTy, _Ty,
    typename std::enable_if<
        std::is_same<decltype(config_bind<_Ty>::from_config(std::declval<const _ConfTy&>())), _Ty>::value, void>::type>
    : std::true_type
{
};

template <typename _ConfTy, typename _Ty, typename _Check = void>
struct is_configor_getable : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct is_configor_getable<
    _ConfTy, _Ty,
    typename std::enable_if<std::is_same<_ConfTy, _Ty>::value || has_default_from_config<_ConfTy, _Ty>::value
                                || has_non_default_from_config<_ConfTy, _Ty>::value,
                            void>::type> : std::true_type
{
};

template <typename _ConfTy, typename _Ty, typename _Check = void>
struct is_configor_setable : std::false_type
{
};

template <typename _ConfTy, typename _Ty>
struct is_configor_setable<
    _ConfTy, _Ty,
    typename std::enable_if<std::is_same<_ConfTy, _Ty>::value || has_to_config<_ConfTy, _Ty>::value, void>::type>
    : std::true_type
{
};

}  // namespace detail

//
// partial specialization of config_bind
//

template <typename _Ty>
struct config_bind<std::unique_ptr<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::unique_ptr<_Ty>& v)
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

    template <typename _ConfTy, typename = typename std::enable_if<is_config<_ConfTy>::value
                                                                   && detail::is_configor_getable<_ConfTy, _Ty>::value
                                                                   && std::is_copy_constructible<_Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::unique_ptr<_Ty>& v)
    {
        if (c.is_null())
        {
            v = nullptr;
        }
        else
        {
            if (v == nullptr)
            {
                v.reset(new _Ty(c.template get<_Ty>()));
            }
            else
            {
                *v = c.template get<_Ty>();
            }
        }
    }
};

template <typename _Ty>
struct config_bind<std::shared_ptr<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::shared_ptr<_Ty>& v)
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

    template <typename _ConfTy, typename = typename std::enable_if<is_config<_ConfTy>::value
                                                                   && detail::is_configor_getable<_ConfTy, _Ty>::value
                                                                   && std::is_copy_constructible<_Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::shared_ptr<_Ty>& v)
    {
        if (c.is_null())
        {
            v = nullptr;
        }
        else
        {
            if (v == nullptr)
            {
                v = std::make_shared<_Ty>(c.template get<_Ty>());
            }
            else
            {
                *v = c.template get<_Ty>();
            }
        }
    }
};

template <typename _Ty, size_t _Num>
struct config_bind<std::array<_Ty, _Num>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::array<_Ty, _Num>& v)
    {
        c = nullptr;
        for (size_t i = 0; i < _Num; i++)
        {
            c[i] = v.at(i);
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::array<_Ty, _Num>& v)
    {
        for (size_t i = 0; i < c.size() && i < _Num; i++)
        {
            v[i] = c[i].template get<_Ty>();
        }
    }
};

template <typename _Ty>
struct config_bind<std::vector<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::vector<_Ty>& v)
    {
        c = nullptr;
        for (size_t i = 0; i < v.size(); ++i)
        {
            c[i] = v.at(i);
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::vector<_Ty>& v)
    {
        v.resize(c.size());
        for (size_t i = 0; i < c.size(); ++i)
        {
            v[i] = c[i].template get<_Ty>();
        }
    }
};

template <typename _Ty>
struct config_bind<std::deque<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::deque<_Ty>& v)
    {
        c = nullptr;
        for (size_t i = 0; i < v.size(); ++i)
        {
            c[i] = v.at(i);
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::deque<_Ty>& v)
    {
        v.resize(c.size());
        for (size_t i = 0; i < c.size(); ++i)
        {
            v[i] = c[i].template get<_Ty>();
        }
    }
};

template <typename _Ty>
struct config_bind<std::list<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::list<_Ty>& v)
    {
        c         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; iter != v.end(); ++i, ++iter)
        {
            c[i] = *iter;
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::list<_Ty>& v)
    {
        v.clear();
        for (size_t i = 0; i < c.size(); i++)
        {
            v.push_back(c[i].template get<_Ty>());
        }
    }
};

template <typename _Ty>
struct config_bind<std::forward_list<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::forward_list<_Ty>& v)
    {
        c         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; iter != v.end(); ++i, ++iter)
        {
            c[i] = *iter;
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::forward_list<_Ty>& v)
    {
        v.clear();
        size_t size = c.size();
        for (size_t i = 0; i < size; i++)
        {
            v.push_front(c[size - i - 1].template get<_Ty>());
        }
    }
};

template <typename _Ty>
struct config_bind<std::set<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::set<_Ty>& v)
    {
        c         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; i < v.size(); ++i, ++iter)
        {
            c[i] = *iter;
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::set<_Ty>& v)
    {
        v.clear();
        for (size_t i = 0; i < c.size(); i++)
        {
            v.insert(c[i].template get<_Ty>());
        }
    }
};

template <typename _Ty>
struct config_bind<std::unordered_set<_Ty>>
{
    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_setable<_ConfTy, _Ty>::value>::type>
    static void to_config(_ConfTy& c, const std::unordered_set<_Ty>& v)
    {
        c         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; i < v.size(); ++i, ++iter)
        {
            c[i] = *iter;
        }
    }

    template <typename _ConfTy,
              typename = typename std::enable_if<is_config<_ConfTy>::value
                                                 && detail::is_configor_getable<_ConfTy, _Ty>::value>::type>
    static void from_config(const _ConfTy& c, std::unordered_set<_Ty>& v)
    {
        v.clear();
        for (size_t i = 0; i < c.size(); i++)
        {
            v.insert(c[i].template get<_Ty>());
        }
    }
};

template <typename _KeyTy, typename _Ty>
struct config_bind<std::map<_KeyTy, _Ty>>
{
    template <
        typename _ConfTy,
        typename std::enable_if<is_config<_ConfTy>::value && std::is_same<_KeyTy, typename _ConfTy::string_type>::value
                                    && detail::is_configor_setable<_ConfTy, _Ty>::value,
                                int>::type = 0>
    static void to_config(_ConfTy& c, const std::map<_KeyTy, _Ty>& v)
    {
        c = nullptr;
        for (const auto& p : v)
        {
            c[p.first] = p.second;
        }
    }

    template <typename _ConfTy, typename = typename std::enable_if<is_config<_ConfTy>::value>::type>
    static void from_config(const _ConfTy& c, std::map<typename _ConfTy::string_type, _ConfTy>& v)
    {
        for (auto iter = c.cbegin(); iter != c.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value());
        }
    }

    template <typename _ConfTy,
              typename std::enable_if<is_config<_ConfTy>::value && !std::is_same<_ConfTy, _Ty>::value
                                          && std::is_constructible<typename _ConfTy::string_type, _KeyTy>::value
                                          && detail::is_configor_getable<_ConfTy, _Ty>::value,
                                      int>::type = 0>
    static void from_config(const _ConfTy& c, std::map<_KeyTy, _Ty>& v)
    {
        for (auto iter = c.cbegin(); iter != c.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value().template get<_Ty>());
        }
    }
};

template <typename _KeyTy, typename _Ty>
struct config_bind<std::unordered_map<_KeyTy, _Ty>>
{
    template <
        typename _ConfTy,
        typename std::enable_if<is_config<_ConfTy>::value && std::is_same<_KeyTy, typename _ConfTy::string_type>::value
                                    && detail::is_configor_setable<_ConfTy, _Ty>::value,
                                int>::type = 0>
    static void to_config(_ConfTy& c, const std::unordered_map<_KeyTy, _Ty>& v)
    {
        c = nullptr;
        for (const auto& p : v)
        {
            c[p.first] = p.second;
        }
    }

    template <typename _ConfTy, typename = typename std::enable_if<is_config<_ConfTy>::value>::type>
    static void from_config(const _ConfTy& c, std::unordered_map<typename _ConfTy::string_type, _ConfTy>& v)
    {
        for (auto iter = c.cbegin(); iter != c.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value());
        }
    }

    template <typename _ConfTy,
              typename std::enable_if<is_config<_ConfTy>::value && !std::is_same<_ConfTy, _Ty>::value
                                          && std::is_constructible<typename _ConfTy::string_type, _KeyTy>::value
                                          && detail::is_configor_getable<_ConfTy, _Ty>::value,
                                      int>::type = 0>
    static void from_config(const _ConfTy& c, std::unordered_map<_KeyTy, _Ty>& v)
    {
        for (auto iter = c.cbegin(); iter != c.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value().template get<_Ty>());
        }
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
