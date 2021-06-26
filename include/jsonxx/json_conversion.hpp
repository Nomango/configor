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
              typename std::enable_if<std::is_void<decltype(::jsonxx::detail::adl_to_json(
                                          std::declval<_JsonTy&>(), std::declval<const _UTy&>()))>::value,
                                      int>::type = 0>
    static inline void to_json(_JsonTy& j, const _UTy& value)
    {
        ::jsonxx::detail::adl_to_json(j, value);
    }

    template <typename _JsonTy, typename _UTy = _Ty,
              typename std::enable_if<std::is_void<decltype(::jsonxx::detail::adl_from_json(
                                          std::declval<const _JsonTy&>(), std::declval<_UTy&>()))>::value,
                                      int>::type = 0>
    static inline void from_json(const _JsonTy& j, _UTy& value)
    {
        ::jsonxx::detail::adl_from_json(j, value);
    }
};

//
// type_traits
//

template <typename _JsonTy, typename _Ty, typename = void>
struct has_to_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_to_json<_JsonTy, _Ty,
                   typename std::enable_if<std::is_void<decltype(json_bind<_Ty>::to_json(
                                               std::declval<_JsonTy&>(), std::declval<const _Ty&>()))>::value,
                                           void>::type> : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename = void>
struct has_default_from_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_default_from_json<_JsonTy, _Ty,
                             typename std::enable_if<std::is_void<decltype(json_bind<_Ty>::from_json(
                                                         std::declval<const _JsonTy&>(), std::declval<_Ty&>()))>::value,
                                                     void>::type> : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename = void>
struct has_non_default_from_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_non_default_from_json<
    _JsonTy, _Ty,
    typename std::enable_if<
        std::is_same<decltype(json_bind<_Ty>::from_json(std::declval<const _JsonTy&>())), _Ty>::value, void>::type>
    : std::true_type
{
};

template <typename _JsonTy, typename _Ty, typename = void>
struct has_from_json : std::false_type
{
};

template <typename _JsonTy, typename _Ty>
struct has_from_json<
    _JsonTy, _Ty,
    typename std::enable_if<
        has_default_from_json<_JsonTy, _Ty>::value || has_non_default_from_json<_JsonTy, _Ty>::value, void>::type>
    : std::true_type
{
};

//
// partial specialization of json_bind
//

template <typename _Ty>
struct json_bind<std::unique_ptr<_Ty>>
{
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
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

    template <typename _JsonTy, typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value
                                                            && std::is_copy_constructible<_Ty>::value,
                                                        int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
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

    template <typename _JsonTy, typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value
                                                            && std::is_copy_constructible<_Ty>::value,
                                                        int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::array<_Ty, _Num>& v)
    {
        j = nullptr;
        for (size_t i = 0; i < _Num; i++)
        {
            j[i] = v.at(i);
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::vector<_Ty>& v)
    {
        j = nullptr;
        for (size_t i = 0; i < v.size(); ++i)
        {
            j[i] = v.at(i);
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::deque<_Ty>& v)
    {
        j = nullptr;
        for (size_t i = 0; i < v.size(); ++i)
        {
            j[i] = v.at(i);
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::list<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; iter != v.end(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::forward_list<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; iter != v.end(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::set<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; i < v.size(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_to_json<_JsonTy, _Ty>::value, int>::type = 0>
    static void to_json(_JsonTy& j, const std::unordered_set<_Ty>& v)
    {
        j         = nullptr;
        auto iter = v.begin();
        for (size_t i = 0; i < v.size(); ++i, ++iter)
        {
            j[i] = *iter;
        }
    }

    template <typename _JsonTy,
              typename std::enable_if<is_json<_JsonTy>::value && has_from_json<_JsonTy, _Ty>::value, int>::type = 0>
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
    template <
        typename _JsonTy,
        typename std::enable_if<is_json<_JsonTy>::value && std::is_same<_KeyTy, typename _JsonTy::string_type>::value
                                    && has_to_json<_JsonTy, _Ty>::value,
                                int>::type = 0>
    static void to_json(_JsonTy& j, const std::map<_KeyTy, _Ty>& v)
    {
        j = nullptr;
        for (const auto& p : v)
        {
            j[p.first] = p.second;
        }
    }

    template <
        typename _JsonTy,
        typename std::enable_if<is_json<_JsonTy>::value && std::is_same<_KeyTy, typename _JsonTy::string_type>::value
                                    && has_from_json<_JsonTy, _Ty>::value,
                                int>::type = 0>
    static void from_json(const _JsonTy& j, std::map<_KeyTy, _Ty>& v)
    {
        for (auto iter = j.cbegin(); iter != j.cend(); iter++)
        {
            v.insert(std::make_pair(iter.key(), iter.value().template get<_Ty>()));
        }
    }
};

template <typename _KeyTy, typename _Ty>
struct json_bind<std::unordered_map<_KeyTy, _Ty>>
{
    template <
        typename _JsonTy,
        typename std::enable_if<is_json<_JsonTy>::value && std::is_same<_KeyTy, typename _JsonTy::string_type>::value
                                    && has_to_json<_JsonTy, _Ty>::value,
                                int>::type = 0>
    static void to_json(_JsonTy& j, const std::unordered_map<_KeyTy, _Ty>& v)
    {
        j = nullptr;
        for (const auto& p : v)
        {
            j[p.first] = p.second;
        }
    }

    template <
        typename _JsonTy,
        typename std::enable_if<is_json<_JsonTy>::value && std::is_same<_KeyTy, typename _JsonTy::string_type>::value
                                    && has_from_json<_JsonTy, _Ty>::value,
                                int>::type = 0>
    static void from_json(const _JsonTy& j, std::unordered_map<_KeyTy, _Ty>& v)
    {
        for (auto iter = j.cbegin(); iter != j.cend(); iter++)
        {
            v.insert(std::make_pair(iter.key(), iter.value().template get<_Ty>()));
        }
    }
};

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

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& out,
                                                     const read_json_wrapper&       wrapper)
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

    friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& in,
                                                     const write_json_wrapper&      wrapper)
    {
        _JsonTy j;
        in >> j;
        const_cast<_Ty&>(wrapper.v_) = j.template get<_Ty>();
        return in;
    }

private:
    _Ty& v_;
};

//
// json_wrap
//

template <typename _Ty, typename _JsonTy = basic_json<>,
          typename std::enable_if<is_json<_JsonTy>::value, int>::type = 0>
inline write_json_wrapper<_Ty, _JsonTy> json_wrap(_Ty& v)
{
    return write_json_wrapper<_Ty, _JsonTy>(v);
}

template <typename _Ty, typename _JsonTy = basic_json<>,
          typename std::enable_if<is_json<_JsonTy>::value, int>::type = 0>
inline read_json_wrapper<_Ty, _JsonTy> json_wrap(const _Ty& v)
{
    return read_json_wrapper<_Ty, _JsonTy>(v);
}

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

}  // namespace detail

}  // namespace jsonxx
