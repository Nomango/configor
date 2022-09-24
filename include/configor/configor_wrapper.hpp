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

namespace configor
{

namespace detail
{
template <typename _ConfigTy, typename _Ty>
class read_wrapper
{
public:
    using config_type = typename _ConfigTy;
    using value_type  = typename _ConfigTy::value;

    explicit read_wrapper(const _Ty& v)
        : v_(v)
    {
    }

    template <typename _TargetCharTy, typename _Traits>
    friend std::basic_ostream<_TargetCharTy, _Traits>& operator<<(std::basic_ostream<_TargetCharTy, _Traits>& out,
                                                                  const read_wrapper&                         wrapper)
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

template <typename _ConfigTy, typename _Ty>
class write_wrapper : public read_wrapper<_ConfigTy, _Ty>
{
public:
    using config_type = typename _ConfigTy;
    using value_type  = typename _ConfigTy::value;

    explicit write_wrapper(_Ty& v)
        : read_wrapper<_ConfigTy, _Ty>(v)
        , v_(v)
    {
    }

    template <typename _SourceCharTy, typename _Traits>
    friend std::basic_istream<_SourceCharTy, _Traits>& operator>>(std::basic_istream<_SourceCharTy, _Traits>& in,
                                                                  write_wrapper&                              wrapper)
    {
        typename std::basic_istream<_SourceCharTy, _Traits>::sentry s(in);
        if (s)
        {
            value_type c{};
            config_type::parse(c, in);
            wrapper.v_ = c.template get<_Ty>();
        }
        return in;
    }

private:
    _Ty& v_;
};
}  // namespace detail

}  // namespace configor
