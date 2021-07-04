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
#include "json_declare.hpp"

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

namespace jsonxx
{

namespace detail
{

template <typename _JsonTy, typename _Ty>
inline auto adl_to_json(_JsonTy& j, const _Ty& value) -> decltype(to_json(j, value))
{
    return to_json(j, value);
}

template <typename _JsonTy, typename _Ty>
inline auto adl_from_json(const _JsonTy& j, _Ty& value) -> decltype(from_json(j, value))
{
    return from_json(j, value);
}

}  // namespace detail

//
// json_bind
//

template <typename _Ty>
struct json_bind
{
    template <typename _JsonTy, typename _UTy = _Ty,
              typename = typename std::enable_if<std::is_void<decltype(::jsonxx::detail::adl_to_json(std::declval<_JsonTy&>(), std::declval<const _UTy&>()))>::value>::type>
    static inline void to_json(_JsonTy& j, const _UTy& value)
    {
        ::jsonxx::detail::adl_to_json(j, value);
    }

    template <typename _JsonTy, typename _UTy = _Ty,
              typename = typename std::enable_if<std::is_void<decltype(::jsonxx::detail::adl_from_json(std::declval<const _JsonTy&>(), std::declval<_UTy&>()))>::value>::type>
    static inline void from_json(const _JsonTy& j, _UTy& value)
    {
        ::jsonxx::detail::adl_from_json(j, value);
    }
};

//
// type_traits
//

namespace detail
{

template <typename _JsonTy, typename _Ty, typename _Check = void>
struct has_to_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_to_json<_JsonTy, _Ty, typename std::enable_if<std::is_void<decltype(json_bind<_Ty>::to_json(std::declval<_JsonTy&>(), std::declval<const _Ty&>()))>::value, void>::type>
    : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename _Check = void>
struct has_default_from_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_default_from_json<_JsonTy, _Ty,
                             typename std::enable_if<std::is_void<decltype(json_bind<_Ty>::from_json(std::declval<const _JsonTy&>(), std::declval<_Ty&>()))>::value, void>::type>
    : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename _Check = void>
struct has_non_default_from_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_non_default_from_json<_JsonTy, _Ty, typename std::enable_if<std::is_same<decltype(json_bind<_Ty>::from_json(std::declval<const _JsonTy&>())), _Ty>::value, void>::type>
    : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename _Check = void>
struct is_json_getable : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct is_json_getable<
    _JsonTy, _Ty,
    typename std::enable_if<std::is_same<_JsonTy, _Ty>::value || has_default_from_json<_JsonTy, _Ty>::value || has_non_default_from_json<_JsonTy, _Ty>::value, void>::type>
    : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename _Check = void>
struct is_json_setable : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct is_json_setable<_JsonTy, _Ty, typename std::enable_if<std::is_same<_JsonTy, _Ty>::value || has_to_json<_JsonTy, _Ty>::value, void>::type> : std::true_type
{
};

}  // namespace detail

//
// partial specialization of json_bind
//

template <typename _Ty>
struct json_bind<std::unique_ptr<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::unique_ptr<_Ty>& v)
    {
        if (v != nullptr)
        {
            j = *v;
        }
        else
        {
            j = nullptr;
        }
    }

    template <typename _JsonTy,
              typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value && std::is_copy_constructible<_Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::unique_ptr<_Ty>& v)
    {
        if (j.is_null())
        {
            v = nullptr;
        }
        else
        {
            if (v == nullptr)
            {
                v.reset(new _Ty(j.template get<_Ty>()));
            }
            else
            {
                *v = j.template get<_Ty>();
            }
        }
    }
};

template <typename _Ty>
struct json_bind<std::shared_ptr<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::shared_ptr<_Ty>& v)
    {
        if (v != nullptr)
        {
            j = *v;
        }
        else
        {
            j = nullptr;
        }
    }

    template <typename _JsonTy,
              typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value && std::is_copy_constructible<_Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::shared_ptr<_Ty>& v)
    {
        if (j.is_null())
        {
            v = nullptr;
        }
        else
        {
            if (v == nullptr)
            {
                v = std::make_shared<_Ty>(j.template get<_Ty>());
            }
            else
            {
                *v = j.template get<_Ty>();
            }
        }
    }
};

template <typename _Ty, size_t _Num>
struct json_bind<std::array<_Ty, _Num>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::array<_Ty, _Num>& v)
    {
        j = nullptr;
        for (size_t i = 0; i < _Num; i++)
        {
            j[i] = v.at(i);
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::array<_Ty, _Num>& v)
    {
        for (size_t i = 0; i < j.size() && i < _Num; i++)
        {
            v[i] = j[i].template get<_Ty>();
        }
    }
};

template <typename _Ty>
struct json_bind<std::vector<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::vector<_Ty>& v)
    {
        j = nullptr;
        for (size_t i = 0; i < v.size(); ++i)
        {
            j[i] = v.at(i);
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::vector<_Ty>& v)
    {
        v.resize(j.size());
        for (size_t i = 0; i < j.size(); ++i)
        {
            v[i] = j[i].template get<_Ty>();
        }
    }
};

template <typename _Ty>
struct json_bind<std::deque<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::deque<_Ty>& v)
    {
        j = nullptr;
        for (size_t i = 0; i < v.size(); ++i)
        {
            j[i] = v.at(i);
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::deque<_Ty>& v)
    {
        v.resize(j.size());
        for (size_t i = 0; i < j.size(); ++i)
        {
            v[i] = j[i].template get<_Ty>();
        }
    }
};

template <typename _Ty>
struct json_bind<std::list<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::list<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; iter != v.end(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::list<_Ty>& v)
    {
        v.clear();
        for (size_t i = 0; i < j.size(); i++)
        {
            v.push_back(j[i].template get<_Ty>());
        }
    }
};

template <typename _Ty>
struct json_bind<std::forward_list<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::forward_list<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; iter != v.end(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::forward_list<_Ty>& v)
    {
        v.clear();
        size_t size = j.size();
        for (size_t i = 0; i < size; i++)
        {
            v.push_front(j[size - i - 1].template get<_Ty>());
        }
    }
};

template <typename _Ty>
struct json_bind<std::set<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::set<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; i < v.size(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::set<_Ty>& v)
    {
        v.clear();
        for (size_t i = 0; i < j.size(); i++)
        {
            v.insert(j[i].template get<_Ty>());
        }
    }
};

template <typename _Ty>
struct json_bind<std::unordered_set<_Ty>>
{
    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_setable<_JsonTy, _Ty>::value>::type>
    static void to_json(_JsonTy& j, const std::unordered_set<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; i < v.size(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value>::type>
    static void from_json(const _JsonTy& j, std::unordered_set<_Ty>& v)
    {
        v.clear();
        for (size_t i = 0; i < j.size(); i++)
        {
            v.insert(j[i].template get<_Ty>());
        }
    }
};

template <typename _KeyTy, typename _Ty>
struct json_bind<std::map<_KeyTy, _Ty>>
{
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && std::is_same<_KeyTy, typename _JsonTy::string_type>::value && detail::is_json_setable<_JsonTy, _Ty>::value,
                                      int>::type = 0>
    static void to_json(_JsonTy& j, const std::map<_KeyTy, _Ty>& v)
    {
        j = nullptr;
        for (const auto& p : v)
        {
            j[p.first] = p.second;
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
    static void from_json(const _JsonTy& j, std::map<typename _JsonTy::string_type, _JsonTy>& v)
    {
        for (auto iter = j.cbegin(); iter != j.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value());
        }
    }

    template <typename _JsonTy, typename std::enable_if<is_json<_JsonTy>::value && !std::is_same<_JsonTy, _Ty>::value
                                                            && std::is_constructible<typename _JsonTy::string_type, _KeyTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value,
                                                        int>::type = 0>
    static void from_json(const _JsonTy& j, std::map<_KeyTy, _Ty>& v)
    {
        for (auto iter = j.cbegin(); iter != j.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value().template get<_Ty>());
        }
    }
};

template <typename _KeyTy, typename _Ty>
struct json_bind<std::unordered_map<_KeyTy, _Ty>>
{
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && std::is_same<_KeyTy, typename _JsonTy::string_type>::value && detail::is_json_setable<_JsonTy, _Ty>::value,
                                      int>::type = 0>
    static void to_json(_JsonTy& j, const std::unordered_map<_KeyTy, _Ty>& v)
    {
        j = nullptr;
        for (const auto& p : v)
        {
            j[p.first] = p.second;
        }
    }

    template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
    static void from_json(const _JsonTy& j, std::unordered_map<typename _JsonTy::string_type, _JsonTy>& v)
    {
        for (auto iter = j.cbegin(); iter != j.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value());
        }
    }

    template <typename _JsonTy, typename std::enable_if<is_json<_JsonTy>::value && !std::is_same<_JsonTy, _Ty>::value
                                                            && std::is_constructible<typename _JsonTy::string_type, _KeyTy>::value && detail::is_json_getable<_JsonTy, _Ty>::value,
                                                        int>::type = 0>
    static void from_json(const _JsonTy& j, std::unordered_map<_KeyTy, _Ty>& v)
    {
        for (auto iter = j.cbegin(); iter != j.cend(); iter++)
        {
            v.emplace(iter.key(), iter.value().template get<_Ty>());
        }
    }
};

namespace detail
{

//
// json_wrapper
//

template <typename _Ty, typename _JsonTy = basic_json<>>
class read_json_wrapper
{
public:
    using char_type = typename _JsonTy::char_type;

    read_json_wrapper(const _Ty& v)
        : v_(v)
    {
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& out, const read_json_wrapper& wrapper)
    {
        out << _JsonTy(wrapper.v_);
        return out;
    }

private:
    const _Ty& v_;
};

template <typename _Ty, typename _JsonTy = basic_json<>>
class write_json_wrapper : public read_json_wrapper<_Ty, _JsonTy>
{
public:
    using char_type = typename _JsonTy::char_type;

    write_json_wrapper(_Ty& v)
        : read_json_wrapper<_Ty, _JsonTy>(v)
        , v_(v)
    {
    }

    friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& in, const write_json_wrapper& wrapper)
    {
        _JsonTy j;
        in >> j;
        const_cast<_Ty&>(wrapper.v_) = j.template get<_Ty>();
        return in;
    }

private:
    _Ty& v_;
};

}  // namespace detail

//
// json_wrap
//

template <typename _Ty, typename _JsonTy = basic_json<>, typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
inline detail::write_json_wrapper<_Ty, _JsonTy> json_wrap(_Ty& v)
{
    return detail::write_json_wrapper<_Ty, _JsonTy>(v);
}

template <typename _Ty, typename _JsonTy = basic_json<>, typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
inline detail::read_json_wrapper<_Ty, _JsonTy> json_wrap(const _Ty& v)
{
    return detail::read_json_wrapper<_Ty, _JsonTy>(v);
}

}  // namespace jsonxx

#define JSONXX_EXPAND(x) x
#define JSONXX_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
                         _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62,  \
                         _63, _64, NAME, ...)                                                                                                                                   \
    NAME
#define JSONXX_PASTE(...)                                                                                                                                                          \
    JSONXX_EXPAND(JSONXX_GET_MACRO(__VA_ARGS__, JSONXX_PASTE64, JSONXX_PASTE63, JSONXX_PASTE62, JSONXX_PASTE61, JSONXX_PASTE60, JSONXX_PASTE59, JSONXX_PASTE58, JSONXX_PASTE57,    \
                                   JSONXX_PASTE56, JSONXX_PASTE55, JSONXX_PASTE54, JSONXX_PASTE53, JSONXX_PASTE52, JSONXX_PASTE51, JSONXX_PASTE50, JSONXX_PASTE49, JSONXX_PASTE48, \
                                   JSONXX_PASTE47, JSONXX_PASTE46, JSONXX_PASTE45, JSONXX_PASTE44, JSONXX_PASTE43, JSONXX_PASTE42, JSONXX_PASTE41, JSONXX_PASTE40, JSONXX_PASTE39, \
                                   JSONXX_PASTE38, JSONXX_PASTE37, JSONXX_PASTE36, JSONXX_PASTE35, JSONXX_PASTE34, JSONXX_PASTE33, JSONXX_PASTE32, JSONXX_PASTE31, JSONXX_PASTE30, \
                                   JSONXX_PASTE29, JSONXX_PASTE28, JSONXX_PASTE27, JSONXX_PASTE26, JSONXX_PASTE25, JSONXX_PASTE24, JSONXX_PASTE23, JSONXX_PASTE22, JSONXX_PASTE21, \
                                   JSONXX_PASTE20, JSONXX_PASTE19, JSONXX_PASTE18, JSONXX_PASTE17, JSONXX_PASTE16, JSONXX_PASTE15, JSONXX_PASTE14, JSONXX_PASTE13, JSONXX_PASTE12, \
                                   JSONXX_PASTE11, JSONXX_PASTE10, JSONXX_PASTE9, JSONXX_PASTE8, JSONXX_PASTE7, JSONXX_PASTE6, JSONXX_PASTE5, JSONXX_PASTE4, JSONXX_PASTE3,        \
                                   JSONXX_PASTE2, JSONXX_PASTE1)(__VA_ARGS__))
#define JSONXX_PASTE2(func, v1) func(v1)
#define JSONXX_PASTE3(func, v1, v2) JSONXX_PASTE2(func, v1) JSONXX_PASTE2(func, v2)
#define JSONXX_PASTE4(func, v1, v2, v3) JSONXX_PASTE2(func, v1) JSONXX_PASTE3(func, v2, v3)
#define JSONXX_PASTE5(func, v1, v2, v3, v4) JSONXX_PASTE2(func, v1) JSONXX_PASTE4(func, v2, v3, v4)
#define JSONXX_PASTE6(func, v1, v2, v3, v4, v5) JSONXX_PASTE2(func, v1) JSONXX_PASTE5(func, v2, v3, v4, v5)
#define JSONXX_PASTE7(func, v1, v2, v3, v4, v5, v6) JSONXX_PASTE2(func, v1) JSONXX_PASTE6(func, v2, v3, v4, v5, v6)
#define JSONXX_PASTE8(func, v1, v2, v3, v4, v5, v6, v7) JSONXX_PASTE2(func, v1) JSONXX_PASTE7(func, v2, v3, v4, v5, v6, v7)
#define JSONXX_PASTE9(func, v1, v2, v3, v4, v5, v6, v7, v8) JSONXX_PASTE2(func, v1) JSONXX_PASTE8(func, v2, v3, v4, v5, v6, v7, v8)
#define JSONXX_PASTE10(func, v1, v2, v3, v4, v5, v6, v7, v8, v9) JSONXX_PASTE2(func, v1) JSONXX_PASTE9(func, v2, v3, v4, v5, v6, v7, v8, v9)
#define JSONXX_PASTE11(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) JSONXX_PASTE2(func, v1) JSONXX_PASTE10(func, v2, v3, v4, v5, v6, v7, v8, v9, v10)
#define JSONXX_PASTE12(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11) JSONXX_PASTE2(func, v1) JSONXX_PASTE11(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11)
#define JSONXX_PASTE13(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12) JSONXX_PASTE2(func, v1) JSONXX_PASTE12(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12)
#define JSONXX_PASTE14(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13) \
    JSONXX_PASTE2(func, v1) JSONXX_PASTE13(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13)
#define JSONXX_PASTE15(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14) \
    JSONXX_PASTE2(func, v1) JSONXX_PASTE14(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14)
#define JSONXX_PASTE16(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15) \
    JSONXX_PASTE2(func, v1) JSONXX_PASTE15(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15)
#define JSONXX_PASTE17(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
    JSONXX_PASTE2(func, v1) JSONXX_PASTE16(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16)
#define JSONXX_PASTE18(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17) \
    JSONXX_PASTE2(func, v1) JSONXX_PASTE17(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17)
#define JSONXX_PASTE19(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18) \
    JSONXX_PASTE2(func, v1)                                                                                   \
    JSONXX_PASTE18(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18)
#define JSONXX_PASTE20(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19) \
    JSONXX_PASTE2(func, v1)                                                                                        \
    JSONXX_PASTE19(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19)
#define JSONXX_PASTE21(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20) \
    JSONXX_PASTE2(func, v1)                                                                                             \
    JSONXX_PASTE20(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20)
#define JSONXX_PASTE22(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21) \
    JSONXX_PASTE2(func, v1)                                                                                                  \
    JSONXX_PASTE21(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21)
#define JSONXX_PASTE23(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22) \
    JSONXX_PASTE2(func, v1)                                                                                                       \
    JSONXX_PASTE22(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22)
#define JSONXX_PASTE24(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23) \
    JSONXX_PASTE2(func, v1)                                                                                                            \
    JSONXX_PASTE23(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23)
#define JSONXX_PASTE25(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24) \
    JSONXX_PASTE2(func, v1)                                                                                                                 \
    JSONXX_PASTE24(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24)
#define JSONXX_PASTE26(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25) \
    JSONXX_PASTE2(func, v1)                                                                                                                      \
    JSONXX_PASTE25(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25)
#define JSONXX_PASTE27(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26) \
    JSONXX_PASTE2(func, v1)                                                                                                                           \
    JSONXX_PASTE26(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26)
#define JSONXX_PASTE28(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27) \
    JSONXX_PASTE2(func, v1)                                                                                                                                \
    JSONXX_PASTE27(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27)
#define JSONXX_PASTE29(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28) \
    JSONXX_PASTE2(func, v1)                                                                                                                                     \
    JSONXX_PASTE28(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28)
#define JSONXX_PASTE30(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29) \
    JSONXX_PASTE2(func, v1)                                                                                                                                          \
    JSONXX_PASTE29(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29)
#define JSONXX_PASTE31(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30) \
    JSONXX_PASTE2(func, v1)                                                                                                                                               \
    JSONXX_PASTE30(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30)
#define JSONXX_PASTE32(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31) \
    JSONXX_PASTE2(func, v1)                                                                                                                                                    \
    JSONXX_PASTE31(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31)
#define JSONXX_PASTE33(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, \
                       v32)                                                                                                                                                    \
    JSONXX_PASTE2(func, v1)                                                                                                                                                    \
    JSONXX_PASTE32(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32)
#define JSONXX_PASTE34(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, \
                       v32, v33)                                                                                                                                               \
    JSONXX_PASTE2(func, v1)                                                                                                                                                    \
    JSONXX_PASTE33(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33)
#define JSONXX_PASTE35(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34)                                                                                                                                            \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE34(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34)
#define JSONXX_PASTE36(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35)                                                                                                                                       \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE35(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35)
#define JSONXX_PASTE37(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36)                                                                                                                                  \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE36(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36)
#define JSONXX_PASTE38(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37)                                                                                                                             \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE37(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37)
#define JSONXX_PASTE39(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38)                                                                                                                        \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE38(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38)
#define JSONXX_PASTE40(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39)                                                                                                                   \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE39(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39)
#define JSONXX_PASTE41(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40)                                                                                                              \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE40(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40)
#define JSONXX_PASTE42(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41)                                                                                                         \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE41(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41)
#define JSONXX_PASTE43(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42)                                                                                                    \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE42(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42)
#define JSONXX_PASTE44(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43)                                                                                               \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE43(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43)
#define JSONXX_PASTE45(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44)                                                                                          \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE44(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44)
#define JSONXX_PASTE46(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45)                                                                                     \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE45(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45)
#define JSONXX_PASTE47(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46)                                                                                \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE46(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46)
#define JSONXX_PASTE48(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47)                                                                           \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE47(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47)
#define JSONXX_PASTE49(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48)                                                                      \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE48(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48)
#define JSONXX_PASTE50(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49)                                                                 \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE49(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49)
#define JSONXX_PASTE51(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50)                                                            \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE50(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50)
#define JSONXX_PASTE52(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51)                                                       \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE51(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51)
#define JSONXX_PASTE53(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52)                                                  \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE52(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52)
#define JSONXX_PASTE54(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53)                                             \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE53(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53)
#define JSONXX_PASTE55(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54)                                        \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE54(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54)
#define JSONXX_PASTE56(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55)                                   \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE55(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55)
#define JSONXX_PASTE57(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56)                              \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE56(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56)
#define JSONXX_PASTE58(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57)                         \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE57(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57)
#define JSONXX_PASTE59(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58)                    \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE58(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58)
#define JSONXX_PASTE60(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59)               \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE59(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59)
#define JSONXX_PASTE61(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60)          \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE60(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60)
#define JSONXX_PASTE62(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,   \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, v61)     \
    JSONXX_PASTE2(func, v1)                                                                                                                                                      \
    JSONXX_PASTE61(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, v61)
#define JSONXX_PASTE63(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,    \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, v61, v62) \
    JSONXX_PASTE2(func, v1)                                                                                                                                                       \
    JSONXX_PASTE62(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33,  \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, v61, v62)
#define JSONXX_PASTE64(func, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31,    \
                       v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, v61, v62, \
                       v63)                                                                                                                                                       \
    JSONXX_PASTE2(func, v1)                                                                                                                                                       \
    JSONXX_PASTE63(func, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33,  \
                   v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, v60, v61, v62, v63)

#define JSONXX_TO_JSON(field) j[#field].operator=(v.field);
#define JSONXX_FROM_JSON(field) j.at(#field).get(v.field);

#define JSONXX_BIND(value_type, ...) JSONXX_BIND_WITH_JSON(basic_json<>, value_type, __VA_ARGS__)

#define JSONXX_BIND_WITH_JSON(json_type, value_type, ...)          \
    friend void to_json(json_type& j, const value_type& v)         \
    {                                                              \
        JSONXX_EXPAND(JSONXX_PASTE(JSONXX_TO_JSON, __VA_ARGS__))   \
    }                                                              \
    friend void from_json(const json_type& j, value_type& v)       \
    {                                                              \
        JSONXX_EXPAND(JSONXX_PASTE(JSONXX_FROM_JSON, __VA_ARGS__)) \
    }
