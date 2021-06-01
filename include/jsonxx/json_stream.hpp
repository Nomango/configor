// Copyright (c) 2021-2022 jsonxx - Nomango
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
#include <istream>      // std::basic_istream
#include <ostream>      // std::basic_ostream
#include <string>       // std::basic_string
#include <type_traits>  // std::char_traits

namespace jsonxx
{

namespace detail
{

template <typename _CharTy>
class ostring_streambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using int_type    = typename std::basic_streambuf<_CharTy>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    ostring_streambuf(string_type& str)
        : str_(str)
    {
    }

protected:
    virtual int_type overflow(int_type c) override
    {
        if (c != EOF)
        {
            str_.push_back(char_traits::to_char_type(c));
        }
        return c;
    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize num) override
    {
        str_.append(s, static_cast<size_t>(num));
        return num;
    }

private:
    string_type& str_;
};

template <typename _CharTy>
class istring_streambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using int_type    = typename std::basic_streambuf<_CharTy>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    istring_streambuf(const string_type& str)
        : str_(str)
        , index_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (index_ >= str_.size())
            return EOF;
        return char_traits::to_int_type(str_[index_]);
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        ++index_;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        const auto copied = str_.copy(s, static_cast<size_t>(num), index_);
        return static_cast<std::streamsize>(copied);
    }

private:
    const string_type& str_;
    size_t             index_;
};

template <typename _CharTy>
class fast_ostringstream : public std::basic_ostream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using string_type = std::basic_string<char_type>;

    fast_ostringstream(string_type& str)
        : std::basic_ostream<_CharTy>(nullptr)
        , buf_(str)
    {
        this->rdbuf(&buf_);
    }

private:
    ostring_streambuf<_CharTy> buf_;
};

template <typename _CharTy>
class fast_istringstream : public std::basic_istream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using string_type = std::basic_string<char_type>;

    fast_istringstream(const string_type& str)
        : std::basic_istream<_CharTy>(nullptr)
        , buf_(str)
    {
        this->rdbuf(&buf_);
    }

private:
    istring_streambuf<_CharTy> buf_;
};

}  // namespace detail

}  // namespace jsonxx
