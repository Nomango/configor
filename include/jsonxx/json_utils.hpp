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
#include <cstdint>  // std::int32_t
#include <string>  // std::string
#include <array>  // std::array
#include <vector>  // std::vector
#include <map>  // std::map
#include <type_traits>  // std::enable_if, std::is_default_constructible

namespace jsonxx
{
    //
    // forward declare
    //

    template <
        template <class _Kty, class _Ty, class... _Args> class _ObjectTy = std::map,
        template <class _Kty, class... _Args> class _ArrayTy = std::vector,
        typename _StringTy = std::string,
        typename _IntegerTy = std::int32_t,
        typename _FloatTy = double,
        typename _BooleanTy = bool,
        template <class _Ty> class _Allocator = std::allocator>
    class basic_json;

#define DECLARE_BASIC_JSON_TEMPLATE                                          \
    template <                                                               \
        template <class _Kty, class _Ty, class... _Args> class _ObjectTy,    \
        template <class _Kty, class... _Args> class _ArrayTy,                \
        typename _StringTy,                                                  \
        typename _IntegerTy,                                                 \
        typename _FloatTy,                                                   \
        typename _BooleanTy,                                                 \
        template <class _Ty> class _Allocator>

#define DECLARE_BASIC_JSON_TPL_ARGS \
    _ObjectTy, _ArrayTy, _StringTy, _IntegerTy, _FloatTy, _BooleanTy, _Allocator

    //
    // is_basic_json
    //

    template <typename>
    struct is_basic_json
        : std::false_type
    {
    };

    DECLARE_BASIC_JSON_TEMPLATE
    struct is_basic_json<basic_json<DECLARE_BASIC_JSON_TPL_ARGS>>
        : std::true_type
    {
    };

    //
    // json_bind
    // Implement the json_bind class in the following format:
    // 
    // template<>
    // struct json_bind<MyClass>
    // {
    //     void to_json(json& j, const MyClass& v) {}
    // 
    //     void from_json(const json& j, MyClass& v) {}
    // };
    //
    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>,
        typename std::enable_if<is_basic_json<_BasicJsonTy>::value, int>::type = 0
    >
    struct json_bind;

    //
    // convert functions
    //

    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>,
        typename std::enable_if<is_basic_json<_BasicJsonTy>::value, int>::type = 0,
        typename std::enable_if<std::is_default_constructible<json_bind<_Ty, _BasicJsonTy>>::value, int>::type = 0
    >
    inline void to_json(_BasicJsonTy& j, const _Ty& value)
    {
        json_bind<_Ty, _BasicJsonTy>().to_json(j, value);
    }

    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>,
        typename std::enable_if<is_basic_json<_BasicJsonTy>::value, int>::type = 0,
        typename std::enable_if<std::is_default_constructible<json_bind<_Ty, _BasicJsonTy>>::value, int>::type = 0
    >
    inline void from_json(const _BasicJsonTy& j, _Ty& value)
    {
        json_bind<_Ty, _BasicJsonTy>().from_json(j, value);
    }

    //
    // json_wrapper
    //

    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>
    >
    class read_json_wrapper
    {
    public:
        using char_type = typename _BasicJsonTy::char_type;

        read_json_wrapper(const _Ty& v) : v_(v) {}

        friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& out, const read_json_wrapper& wrapper)
        {
            _BasicJsonTy j;
            ::jsonxx::to_json(j, wrapper.v_);
            out << j;
            return out;
        }

    private:
        const _Ty& v_;
    };

    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>
    >
    class write_json_wrapper : public read_json_wrapper<_Ty, _BasicJsonTy>
    {
    public:
        using char_type = typename _BasicJsonTy::char_type;

        write_json_wrapper(_Ty& v) : read_json_wrapper<_Ty, _BasicJsonTy>(v), v_(v) {}

        friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& in, const write_json_wrapper& wrapper)
        {
            _BasicJsonTy j;
            in >> j;
            ::jsonxx::from_json(j, const_cast<_Ty&>(wrapper.v_));
            return in;
        }

    private:
        _Ty& v_;
    };

    //
    // json_wrap
    //

    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>,
        typename std::enable_if<is_basic_json<_BasicJsonTy>::value, int>::type = 0,
        typename std::enable_if<std::is_default_constructible<json_bind<_Ty, _BasicJsonTy>>::value, int>::type = 0
    >
    inline write_json_wrapper<_Ty, _BasicJsonTy> json_wrap(_Ty& v)
    {
        return write_json_wrapper<_Ty, _BasicJsonTy>(v);
    }

    template <
        typename _Ty,
        typename _BasicJsonTy = basic_json<>,
        typename std::enable_if<is_basic_json<_BasicJsonTy>::value, int>::type = 0,
        typename std::enable_if<std::is_default_constructible<json_bind<_Ty, _BasicJsonTy>>::value, int>::type = 0
    >
    inline read_json_wrapper<_Ty, _BasicJsonTy> json_wrap(const _Ty& v)
    {
        return read_json_wrapper<_Ty, _BasicJsonTy>(v);
    }

} // namespace jsonxx
