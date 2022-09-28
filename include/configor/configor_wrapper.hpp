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
#include "configor_parser.hpp"
#include "configor_serializer.hpp"

namespace configor
{

namespace detail
{
template <typename _Args, typename _ValTy>
class iovalue_data : public basic_value_data<_Args, _ValTy>
{
    using inheritance_type = basic_value_data<_Args, _ValTy>;
    using value_type       = _ValTy;

    // template <typename _CharTy>
    // using string_type = typename value_type::tplargs_type::template string_type<_CharTy>;

public:
    iovalue_data()
        : inheritance_type{}
    {
    }

    iovalue_data(const value_base::value_type t)
        : inheritance_type{ t }
    {
    }

    iovalue_data(const iovalue_data& other)
        : inheritance_type{ other }
    {
    }

    iovalue_data(iovalue_data&& other) noexcept
        : inheritance_type{ std::move(other) }
    {
    }

    iovalue_data& operator=(const iovalue_data& other)
    {
        inheritance_type::operator=(other);
        return *this;
    }

    iovalue_data& operator=(iovalue_data&& other)
    {
        inheritance_type::operator=(std::move(other));
        return *this;
    }

    // template <typename _TargetCharTy = typename value_type::char_type>
    // string_type<_TargetCharTy> dump(std::initializer_list<serializer_option<_TargetCharTy>> options = {}) const
    // {
    //     string_type<_TargetCharTy> result;
    //     dump<_TargetCharTy>(result, v, options);
    //     return result;
    // }
};

template <typename _ConfigTy, typename _Ty>
class ostream_wrapper
{
public:
    using config_type = _ConfigTy;
    using value_type  = typename _ConfigTy::value;

    explicit ostream_wrapper(const _Ty& v)
        : v_(v)
    {
    }

    template <typename _TargetCharTy, typename _Traits>
    friend std::basic_ostream<_TargetCharTy, _Traits>& operator<<(std::basic_ostream<_TargetCharTy, _Traits>& out,
                                                                  const ostream_wrapper&                      wrapper)
    {
        typename std::basic_ostream<_TargetCharTy, _Traits>::sentry s(out);
        if (s)
        {
            config_type::dump(out, static_cast<value_type>(wrapper.v_));
        }
        return out;
    }

private:
    const _Ty& v_;
};

template <typename _ConfigTy, typename _Args>
class ostream_wrapper<_ConfigTy, basic_value<_Args>>
{
public:
    using config_type = _ConfigTy;
    using value_type  = typename _ConfigTy::value;

    explicit ostream_wrapper(const value_type& v)
        : v_(v)
    {
    }

    template <typename _TargetCharTy, typename _Traits>
    friend std::basic_ostream<_TargetCharTy, _Traits>& operator<<(std::basic_ostream<_TargetCharTy, _Traits>& out,
                                                                  const ostream_wrapper&                      wrapper)
    {
        typename std::basic_ostream<_TargetCharTy, _Traits>::sentry s(out);
        if (s)
        {
            config_type::dump(out, wrapper.v_);
        }
        return out;
    }

private:
    const value_type& v_;
};

template <typename _ConfigTy, typename _Ty>
class iostream_wrapper : public ostream_wrapper<_ConfigTy, _Ty>
{
public:
    using config_type = _ConfigTy;
    using value_type  = typename _ConfigTy::value;

    explicit iostream_wrapper(_Ty& v)
        : ostream_wrapper<_ConfigTy, _Ty>(v)
        , v_(v)
    {
    }

    template <typename _SourceCharTy, typename _Traits>
    friend std::basic_istream<_SourceCharTy, _Traits>&
    operator>>(std::basic_istream<_SourceCharTy, _Traits>& in,
               const iostream_wrapper&                     wrapper /* wrapper must be lvalue */)
    {
        typename std::basic_istream<_SourceCharTy, _Traits>::sentry s(in);
        if (s)
        {
            value_type c{};
            config_type::parse(c, in);
            const_cast<_Ty&>(wrapper.v_) = c.template get<_Ty>();
        }
        return in;
    }

private:
    _Ty& v_;
};

template <typename _ConfigTy, typename _Args>
class iostream_wrapper<_ConfigTy, basic_value<_Args>> : public ostream_wrapper<_ConfigTy, basic_value<_Args>>
{
public:
    using config_type = _ConfigTy;
    using value_type  = typename _ConfigTy::value;

    explicit iostream_wrapper(value_type& v)
        : ostream_wrapper<_ConfigTy, value_type>(v)
        , v_(v)
    {
    }

    template <typename _SourceCharTy, typename _Traits>
    friend std::basic_istream<_SourceCharTy, _Traits>&
    operator>>(std::basic_istream<_SourceCharTy, _Traits>& in,
               const iostream_wrapper&                     wrapper /* wrapper must be lvalue */)
    {
        typename std::basic_istream<_SourceCharTy, _Traits>::sentry s(in);
        if (s)
        {
            config_type::parse(const_cast<value_type&>(wrapper.v_), in);
        }
        return in;
    }

private:
    value_type& v_;
};

template <typename _ConfigTy, typename _ValTy>
class iostream_wrapper_maker
{
public:
    using config_type = _ConfigTy;
    using value_type  = _ValTy;

    template <typename _Ty, typename = typename std::enable_if<std::is_same<value_type, _Ty>::value
                                                               || has_to_value<value_type, _Ty>::value>::type>
    static inline ostream_wrapper<config_type, _Ty> wrap(const _Ty& v)
    {
        return ostream_wrapper<config_type, _Ty>(v);
    }

    template <typename _Ty, typename = typename std::enable_if<(std::is_same<value_type, _Ty>::value
                                                                || is_value_getable<value_type, _Ty>::value)
                                                               && !std::is_pointer<_Ty>::value>::type>
    static inline iostream_wrapper<config_type, _Ty> wrap(_Ty& v)
    {
        return iostream_wrapper<config_type, _Ty>(v);
    }
};
}  // namespace detail

}  // namespace configor
