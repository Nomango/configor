// Copyright (c) 2018-2022 configor - Nomango
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
#include "configor_token.hpp"

namespace configor
{

namespace detail
{

template <typename _IntTy, typename _FloatTy, typename _StrTy>
class token
{
public:
    using integer_type = _IntTy;
    using float_type   = _FloatTy;
    using string_type  = _StrTy;

    token_type type() const
    {
        return type_;
    }

    virtual void get_integer(integer_type& out) = 0;
    virtual void get_float(float_type& out)     = 0;
    virtual void get_string(string_type& out)   = 0;

private:
    token_type type_;
};

// tokenbuf
template <typename _TokenTy>
class basic_tokenbuf
{
public:
    using token_type = _TokenTy;

    virtual token_type& underflow()                    = 0;
    virtual token_type& underflow(const token_type& t) = 0;
};

// tokenization

template <typename _TokenTy>
class basic_otokenization
{
public:
    basic_otokenization(basic_tokenbuf<_TokenTy>* buf)
        : buf_(buf)
    {
    }

private:
    basic_tokenbuf<_TokenTy>* buf_;
};

template <typename _TokenTy>
class basic_itokenization
{
public:
    basic_itokenization(basic_tokenbuf<_TokenTy>* buf)
        : buf_(buf)
    {
    }

private:
    basic_tokenbuf<_TokenTy>* buf_;
};

// value tokenization
template <typename _ValTy, typename _TokenTy>
class basic_value_tokenbuf : public basic_tokenbuf<_TokenTy>
{
public:
    using token_type = _TokenTy;
    using value_type = _ValTy;

    basic_value_tokenbuf(value_type& v)
        : val_(v)
    {
    }

    virtual token_type& underflow()                    = 0;
    virtual token_type& underflow(const token_type& t) = 0;

private:
    value_type&                               val_;
    std::stack<typename value_type::iterator> iters_;
};

template <typename _ValTy, typename _TokenTy>
class basic_value_itokenization : public basic_itokenization<_TokenTy>
{
public:
    basic_value_itokenization() {}

private:
    basic_value_tokenbuf<_ValTy, _TokenTy> buf_;
};

// iterator

template <typename _TokenTy>
class otoken_iterator
{
public:
    using iterator_category  = std::output_iterator_tag;
    using value_type         = void;
    using difference_type    = void;
    using pointer            = void;
    using reference          = void;
    using token_type         = _TokenTy;
    using otokenization_type = basic_otokenization<_TokenTy>;

    otoken_iterator(otokenization_type& out) noexcept
        : out_(std::addressof(out))
    {
    }

    otoken_iterator& operator=(const token_type& t)
    {
        *out_ << t;
        return *this;
    }

    otoken_iterator& operator*() noexcept
    {
        return *this;
    }

    otoken_iterator& operator++() noexcept
    {
        return *this;
    }

    otoken_iterator& operator++(int) noexcept
    {
        return *this;
    }

private:
    otokenization_type* out_;
};

template <typename _TokenTy>
class itoken_iterator
{
public:
    using iterator_category  = std::input_iterator_tag;
    using value_type         = _TokenTy;
    using difference_type    = std::ptrdiff_t;
    using pointer            = const _TokenTy*;
    using reference          = const _TokenTy&;
    using token_type         = _TokenTy;
    using itokenization_type = basic_itokenization<_TokenTy>;

    itoken_iterator() {}

    itoken_iterator(itokenization_type& in)
        : in_(std::addressof(in))
    {
        read_token();
    }

    const token_type& operator*() const noexcept
    {
        return token_;
    }

    const token_type* operator->() const noexcept
    {
        return std::addressof(token_);
    }

    itoken_iterator& operator++()
    {
        read_token();
        return *this;
    }

    itoken_iterator operator++(int)
    {
        itoken_iterator tmp = *this;
        read_token();
        return tmp;
    }

    friend bool operator==(const itoken_iterator& lhs, const itoken_iterator& rhs)
    {
        return lhs.in_ == rhs.in_;
    }

private:
    void read_token()
    {
        if (!(*in_ >> token_))
        {
            in_ = nullptr;
        }
    }

    itokenization_type* in_{ nullptr };
    token_type          token_{};
};

}  // namespace detail

}  // namespace configor
