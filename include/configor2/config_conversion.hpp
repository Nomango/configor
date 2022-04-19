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
#include "config_declare.hpp"
#include "config_parser.hpp"
#include "config_serializer.hpp"

#include <array>         // std::array
#include <deque>         // std::deque
#include <forward_list>  // std::forward_list
#include <functional>    // std::reference_wrapper
#include <list>          // std::list
#include <map>           // std::map
#include <memory>        // std::unique_ptr, std::shared_ptr, std::make_shared
#include <queue>         // std::queue
#include <set>           // std::set
#include <type_traits>   // std::enable_if, std::is_same, std::false_type, std::true_type, std::is_void
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <utility>       // std::forward, std::declval
#include <vector>        // std::vector

namespace configor {

//
// type_traits
//

namespace detail {

// template <typename Config, typename... _Args>
// struct can_serialize {
// private:
//     using serializer_type = typename Config::template serializer<>;
//     using char_type       = typename Config::char_type;
//     using ostream_type    = std::basic_ostream<char_type>;

//     template <typename _UTy, typename... _UArgs>
//     using dump_fn = decltype(_UTy::dump(std::declval<_UArgs>()...));

// public:
//     static constexpr bool value = is_detected<dump_fn, serializer_type, Config, ostream_type&, _Args...>::value;
// };

// template <typename Config, typename... _Args>
// struct can_parse {
// private:
//     using parser_type  = typename Config::template parser<>;
//     using char_type    = typename Config::char_type;
//     using istream_type = std::basic_istream<char_type>;

//     template <typename _UTy, typename... _UArgs>
//     using parse_fn = decltype(_UTy::parse(std::declval<_UArgs>()...));

// public:
//     static constexpr bool value = is_detected<parse_fn, parser_type, Config&, istream_type&, _Args...>::value;
// };

// template <typename ValueT, typename T, typename _Void = void>
// struct has_from_config : std::false_type {};

// template <typename ValueT, typename T>
// struct has_from_config<ValueT, T, typename std::enable_if<!is_config<T>::value>::type> {
// private:
//     using binder_type = typename ValueT::template binder_type<T>;

//     template <typename _UTy, typename... _Args>
//     using from_config_fn = decltype(_UTy::do_parse(std::declval<_Args>()...));

// public:
//     static constexpr bool value = exact_detect<void, from_config_fn, binder_type, ValueT, T&>::value;
// };

// template <typename ValueT, typename T, typename _Void = void>
// struct has_non_default_from_config : std::false_type {};

// template <typename ValueT, typename T>
// struct has_non_default_from_config<ValueT, T, typename std::enable_if<!is_config<T>::value>::type> {
// private:
//     using binder_type = typename ValueT::template binder_type<T>;

//     template <typename _UTy, typename... _Args>
//     using from_config_fn = decltype(_UTy::do_parse(std::declval<_Args>()...));

// public:
//     static constexpr bool value = exact_detect<T, from_config_fn, binder_type, ValueT>::value;
// };

// do_serialize functions

template <typename ValueT, typename T,
          typename std::enable_if<std::is_same<T, typename ValueT::boolean_type>::value, int>::type = 0>
void do_serialize(basic_serializer<ValueT>& s, T v) {
    s.next(token_type::value_boolean);
    s.put_boolean(v);
}

template <typename ValueT, typename T,
          typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, typename ValueT::boolean_type>::value,
                                  int>::type = 0>
void do_serialize(basic_serializer<ValueT>& s, T v) {
    s.next(token_type::value_integer);
    s.put_integer(static_cast<typename ValueT::integer_type>(v));
}

template <typename ValueT, typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void do_serialize(basic_serializer<ValueT>& s, T v) {
    s.next(token_type::value_float);
    s.put_integer(static_cast<typename ValueT::float_type>(v));
}

template <typename ValueT>
void do_serialize(basic_serializer<ValueT>& s, const typename ValueT::string_type& v) {
    s.next(token_type::value_string);
    s.put_string(v);
}

template <typename ValueT>
void do_serialize(basic_serializer<ValueT>& s, typename ValueT::string_type&& v) {
    s.next(token_type::value_string);
    s.put_string(std::move(v));
}

template <typename ValueT, typename T,
          typename std::enable_if<std::is_constructible<typename ValueT::string_type, T>::value &&
                                      !std::is_same<T, typename ValueT::string_type>::value,
                                  int>::type = 0>
void do_serialize(basic_serializer<ValueT>& s, const T& v) {
    s.next(token_type::value_string);
    s.put_string(v);
}

// template <typename ValueT, typename T,
//           typename std::enable_if<std::is_same<T, typename ValueT::array_type>::value, int>::type = 0>
// void do_serialize(basic_serializer<ValueT>& s, const T& v) {
//     s.next(token_type::begin_array, v.size());
//     for (const auto& elem : v) {
//         do_serialize(s, v);
//     }
//     s.next(token_type::end_array);
// }

// template <typename ValueT, typename T,
//           typename std::enable_if<std::is_same<T, typename ValueT::object_type>::value, int>::type = 0>
// void do_serialize(basic_serializer<ValueT>& s, const T& v) {
//     s.next(token_type::begin_object, v.size());
//     for (const auto& pair : v) {
//         do_serialize(s, pair.first);
//         do_serialize(s, pair.second);
//     }
//     s.next(token_type::end_object);
// }

// template <typename ValueT>
// void do_serialize(basic_serializer<ValueT>& s, const ValueT& v) {
//     switch (v.type()) {
//     case config_value_type::object:
//         do_serialize(s, *v.raw_value().object);
//         break;
//     case config_value_type::array:
//         do_serialize(s, *v.raw_value().array);
//         break;
//     case config_value_type::string:
//         s.next(token_type::value_string);
//         s.put_string(*c.raw_value().string);
//         break;
//     case config_value_type::number_integer:
//         s.next(token_type::value_integer);
//         s.put_integer(c.raw_value().number_integer);
//         break;
//     case config_value_type::number_float:
//         s.next(token_type::value_float);
//         s.put_float(c.raw_value().number_float);
//         break;
//     case config_value_type::boolean:
//         s.next(token_type::value_boolean);
//         s.put_boolean(c.raw_value().boolean);
//         break;
//     default:
//         s.next(token_type::value_null);
//         break;
//     }
// }

// do_parse functions

inline auto make_deserialization_error(token_type actual, const std::string& msg = "unexpected token") {
    return bad_deserialization(msg + " '" + to_string(actual) + "'");
}

inline auto make_deserialization_error(token_type actual, token_type expect,
                                       const std::string& msg = "unexpected token") {
    return bad_deserialization(msg + ", expects '" + to_string(expect) + "', but got");
}

template <typename ValueT>
void must_take(basic_parser<ValueT>& p, token_type last, const token_type expect) {
    if (last == token_type::none)
        last = p.scan();
    if (last != expect)
        throw make_deserialization_error(last, expect);
}

template <typename ValueT, typename T,
          typename std::enable_if<std::is_same<T, typename ValueT::boolean_type>::value, int>::type = 0>
void do_parse(basic_parser<ValueT>& p, T& v, token_type last = token_type::none) {
    must_take(p, last, token_type::value_boolean);
    p.get_boolean(v);
}

template <typename ValueT, typename T,
          typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, typename ValueT::boolean_type>::value,
                                  int>::type = 0>
void do_parse(basic_parser<ValueT>& p, T& v, token_type last = token_type::none) {
    must_take(p, last, token_type::value_integer);
    p.get_integer(v);
}

template <typename ValueT, typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void do_parse(basic_parser<ValueT>& p, T& v, token_type last = token_type::none) {
    must_take(p, last, token_type::value_float);
    p.get_float(v);
}

template <typename ValueT>
void do_parse(basic_parser<ValueT>& p, typename ValueT::string_type& v, token_type last = token_type::none) {
    must_take(p, last, token_type::value_string);
    p.get_string(v);
}

template <typename ValueT, typename T,
          typename std::enable_if<std::is_constructible<T, typename ValueT::string_type>::value &&
                                      !std::is_same<T, typename ValueT::string_type>::value,
                                  int>::type = 0>
void do_parse(basic_parser<ValueT>& p, T& v, token_type last = token_type::none) {
    must_take(p, last, token_type::value_string);

    using string_type = typename ValueT::string_type;
    string_type s;
    p.get_string(s);
    v = std::move(s);
}

// template <typename ValueT, typename T>
// void do_parse_array_impl(basic_parser<ValueT>& p, T& v, token_type last) {
//     if (last == token_type::none)
//         last = p.scan();
//     if (last == token_type::value_null) {
//         v.clear(); // FIXME t应该是任意类型
//         return;
//     }
//     if (last != token_type::begin_array)
//         throw make_deserialization_error(last, token_type::begin_array);

//     while (true) {
//         const auto token = p.scan();
//         if (token == token_type::end_array)
//             break;
//         v.push_back({}); // FIXME T不一定有默认构造
//         do_parse(p, v.back(), token);
//     }
// }

// template <typename ValueT, typename T,
//           typename std::enable_if<std::is_same<T, typename ValueT::array_type>::value, int>::type = 0>
// void do_parse(basic_parser<ValueT>& p, T& v, token_type last = token_type::none) {
//     // TODO T不一定是array_type
//     do_parse_array_impl(p, v, last);
// }

// template <typename ValueT, typename T>
// void do_parse_object_impl(basic_parser<ValueT>& p, T& v, token_type last) {
//     if (last == token_type::none)
//         last = p.scan();
//     if (last == token_type::value_null) {
//         v.clear(); // FIXME t应该是任意类型
//         return;
//     }
//     if (last != token_type::begin_object)
//         throw make_deserialization_error(last, token_type::begin_object);

//     using key_type    = typename T::key_type;
//     using mapped_type = typename T::mapped_type;
//     while (true) {
//         const auto token = p.scan();
//         if (token == token_type::end_object)
//             break;
//         // read key
//         if (token != token_type::value_string)
//             throw make_deserialization_error(last, token_type::value_string);

//         key_type    key;
//         mapped_type value; // FIXME T不一定有默认构造
//         do_parse(p, key, token);
//         do_parse(p, value);
//         v.emplace(std::move(key), std::move(value));
//     }
// }

// template <typename ValueT, typename T,
//           typename std::enable_if<std::is_same<T, typename ValueT::object_type>::value, int>::type = 0>
// void do_parse(basic_parser<ValueT>& p, T& v, token_type last = token_type::none) {
//     // TODO T不一定是object_type
//     do_parse_object_impl(p, v, last);
// }

// template <typename ValueT>
// void do_parse(basic_parser<ValueT>& p, ValueT& v, token_type last = token_type::none) {
//     auto token = last;
//     if (token == token_type::none)
//         token = p.scan();

//     switch (token) {
//     case token_type::begin_object:
//         v = config_value_type::object;
//         do_parse(p, *v.raw_value().object);
//         break;
//     case token_type::begin_array:
//         v = config_value_type::array;
//         do_parse(p, *v.raw_value().array);
//         break;
//     case token_type::value_string:
//         v = config_value_type::string;
//         p.get_string(*v.raw_value().string);
//         break;
//     case token_type::value_integer:
//         v = config_value_type::number_integer;
//         p.get_integer(v.raw_value().number_integer);
//         break;
//     case token_type::value_float:
//         v = config_value_type::number_float;
//         p.get_float(v.raw_value().number_float);
//         break;
//     case token_type::value_boolean:
//         v = config_value_type::boolean;
//         p.get_boolean(v.raw_value().boolean);
//         break;
//     case token_type::value_null:
//         v = config_value_type::null;
//         break;
//     default:
//         throw make_deserialization_error(token);
//     }
// }

template <typename ValueT, typename T>
struct has_do_serialize {
private:
    using binder_type = typename ValueT::template binder_type<T>;

    template <typename U, typename... Args>
    using do_serialize_fn = decltype(U::do_serialize(std::declval<Args>()...));

public:
    static constexpr bool value = exact_detect<void, do_serialize_fn, binder_type, ValueT&, T>::value;
};

template <typename ValueT, typename T>
struct has_do_parse {
private:
    using binder_type = typename ValueT::template binder_type<T>;

    template <typename U, typename... Args>
    using do_parse_fn = decltype(U::do_parse(std::declval<Args>()...));

public:
    static constexpr bool value = exact_detect<void, do_parse_fn, binder_type, ValueT&, T>::value;
};

template <typename ValueT>
class transfer_to {
public:
    using value_type      = ValueT;
    using string_type     = typename ValueT::string_type;
    using size_type       = typename ValueT::size_type;
    using serializer_type = basic_serializer<value_type>;
    using parser_type     = basic_parser<value_type>;

    // serialize functions

    template <class T, class U = typename remove_cvref<T>::type,
              class = typename std::enable_if<has_do_serialize<value_type, U>::value>::type>
    void set(T&& v) {
        do_serialize(serializer_.get(), std::forward<T>(v));
    }

    void begin_array(size_type size = 0) { serializer_.get().next(token_type::begin_array, size); }

    void end_array() { serializer_.get().next(token_type::end_array); }

    void begin_object(size_type size = 0) { serializer_.get().next(token_type::begin_object, size); }

    void end_object() { serializer_.get().next(token_type::end_object); }

    transfer operator[](const string_type& key) {}

private:
    std::reference_wrapper<serializer_type> serializer_;
};

template <typename ValueT>
class transfer_from {
public:
    using value_type      = ValueT;
    using string_type     = typename ValueT::string_type;
    using size_type       = typename ValueT::size_type;
    using serializer_type = basic_serializer<value_type>;
    using parser_type     = basic_parser<value_type>;

    // deserialize functions

    template <class T, class U = typename remove_cvref<T>::type,
              class = typename std::enable_if<has_do_parse<value_type, U>::value>::type>
    void get(T& v) {
        do_parse(parser_.get(), std::forward<T>(v));
    }

    transfer operator[](const string_type& key) const {}

private:
    std::reference_wrapper<parser_type> parser_;
};

template <class ValueT, class T> // FIXME check T
void transfer(transfer_to<ValueT>& t, const std::vector<T>& v) {
    t.begin_array(v.size());
    const auto size = v.size();
    for (decltype(size) i = 0; i < size; ++i) {
        t[i].set(v[i]);
    }
    t.end_array();
}

template <class ValueT, class T, class Key = typename ValueT::string_type> // FIXME check T & Key
void transfer(transfer_to<ValueT>& t, const std::map<Key, T>& v) {
    t.begin_object();
    for (const auto& elem : v) {
        t[elem.first].set(elem.second);
    }
    t.end_object();
}

template <class ValueT, class T> // FIXME check T
void transfer(transfer_from<ValueT>& t, std::vector<T>& v) {
    v.clear();
    for (const auto& elem : t) {
        v.push_back(elem.template get<T>());
    }
}

template <class ValueT, class T, class Key = typename ValueT::string_type> // FIXME check T & Key
void transfer(transfer_from<ValueT>& t, const std::map<Key, T>& v) {
    v.clear();
    for (const auto& elem : t) {
        v.emplace(elem.key(), elem.value().template get<T>());
    }
}

} // namespace configor
