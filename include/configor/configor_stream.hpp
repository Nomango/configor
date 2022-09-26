// Copyright (c) 2021-2022 configor - Nomango
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
#include <algorithm>    // std::none_of
#include <cmath>        // std::ceil, std::floor
#include <cstdio>       // std::FILE, std::fgetc, std::fgets, std::fgetwc, std::fgetws, std::snprintf
#include <iomanip>      // std::fill, std::setw
#include <ios>          // std::basic_ios, std::streamsize, std::hex, std::dec, std::uppercase, std::nouppercase
#include <istream>      // std::basic_istream
#include <ostream>      // std::basic_ostream
#include <streambuf>    // std::streambuf
#include <string>       // std::basic_string
#include <type_traits>  // std::char_traits
#include <utility>      // std::forward

namespace configor
{

//
// basic_oadapter
//

template <typename _CharTy>
struct basic_oadapter
{
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;

    virtual void write(const char_type ch) = 0;

    virtual void write(const char_type* str, std::size_t size)
    {
        if (size)
        {
            for (std::size_t i = 0; i < size; ++i)
                this->write(str[i]);
        }
    }
};

using oadapter  = basic_oadapter<char>;
using woadapter = basic_oadapter<wchar_t>;

template <typename _CharTy>
class basic_oadapterstream : public std::basic_ostream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using traits_type = typename std::basic_ostream<char_type>::traits_type;
    using int_type    = typename std::basic_ostream<char_type>::int_type;

    explicit basic_oadapterstream(basic_oadapter<char_type>& adapter)
        : std::basic_ostream<char_type>(nullptr)
        , buf_(adapter)
    {
        this->rdbuf(&buf_);
    }

private:
    class streambuf : public std::basic_streambuf<char_type>
    {
    public:
        explicit streambuf(basic_oadapter<char_type>& adapter)
            : adapter_(adapter)
        {
        }

    protected:
        virtual int_type overflow(int_type c) override
        {
            if (c != EOF)
            {
                adapter_.write(traits_type::to_char_type(c));
            }
            return c;
        }

        virtual std::streamsize xsputn(const char_type* s, std::streamsize num) override
        {
            adapter_.write(s, static_cast<size_t>(num));
            return num;
        }

    private:
        basic_oadapter<char_type>& adapter_;
    };

    streambuf buf_;
};

using oadapterstream  = basic_oadapterstream<char>;
using woadapterstream = basic_oadapterstream<wchar_t>;

//
// basic_iadapter
//

template <typename _CharTy>
struct basic_iadapter
{
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;

    virtual char_type read() = 0;

    virtual void read(char_type* s, size_t num)
    {
        if (num)
        {
            for (std::size_t i = 0; i < num; ++i)
                s[i] = this->read();
        }
    }
};

using iadapter  = basic_iadapter<char>;
using wiadapter = basic_iadapter<wchar_t>;

template <typename _CharTy>
class basic_iadapterstream : public std::basic_istream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using traits_type = typename std::basic_istream<char_type>::traits_type;
    using int_type    = typename std::basic_istream<char_type>::int_type;

    explicit basic_iadapterstream(basic_iadapter<char_type>& adapter)
        : std::basic_istream<_CharTy>(nullptr)
        , buf_(adapter)
    {
        this->rdbuf(&buf_);
    }

private:
    class streambuf : public std::basic_streambuf<char_type>
    {
    public:
        explicit streambuf(basic_iadapter<char_type>& adapter)
            : adapter_(adapter)
            , last_char_(0)
        {
        }

    protected:
        virtual int_type underflow() override
        {
            if (last_char_ != 0)
                return last_char_;
            last_char_ = traits_type::to_char_type(adapter_.read());
            return last_char_;
        }

        virtual int_type uflow() override
        {
            int_type c = underflow();
            last_char_ = 0;
            return c;
        }

        virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
        {
            if (last_char_ != 0)
            {
                // read last char
                s[0]       = traits_type::to_char_type(last_char_);
                last_char_ = 0;
                ++s;
                --num;
            }
            adapter_.read(s, static_cast<size_t>(num));
            return num;
        }

    private:
        basic_iadapter<char_type>& adapter_;
        int_type                   last_char_;
    };

    streambuf buf_;
};

using iadapterstream  = basic_iadapterstream<char>;
using wiadapterstream = basic_iadapterstream<wchar_t>;

namespace detail
{

//
// ostreambuf
//

template <typename _CharTy>
class fast_string_ostreambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using traits_type = typename std::basic_streambuf<char_type>::traits_type;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using string_type = std::basic_string<char_type>;

    explicit fast_string_ostreambuf(string_type& str)
        : str_(str)
    {
    }

protected:
    virtual int_type overflow(int_type c) override
    {
        if (c != traits_type::eof())
        {
            str_.push_back(traits_type::to_char_type(c));
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

//
// istreambuf
//

template <typename _CharTy>
class fast_string_istreambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using traits_type = typename std::basic_streambuf<char_type>::traits_type;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using string_type = std::basic_string<char_type>;

    explicit fast_string_istreambuf(const string_type& str)
        : str_(str)
        , index_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (index_ >= str_.size())
            return traits_type::eof();
        return traits_type::to_int_type(str_[index_]);
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
class fast_buffer_istreambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using traits_type = typename std::basic_streambuf<char_type>::traits_type;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using string_type = std::basic_string<char_type>;

    explicit fast_buffer_istreambuf(const char_type* buffer)
        : buffer_(buffer)
        , index_(0)
        , size_(traits_type::length(buffer))
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (index_ >= size_)
            return traits_type::eof();
        return traits_type::to_int_type(buffer_[index_]);
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        ++index_;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        if (index_ + static_cast<size_t>(num) >= size_)
            num = static_cast<std::streamsize>(size_ - index_);
        if (num == 0)
            return 0;
        traits_type::copy(s, buffer_, static_cast<size_t>(num));
        return num;
    }

private:
    const char_type* buffer_;
    size_t           index_;
    const size_t     size_;
};

template <typename _CharTy>
class fast_cfile_istreambuf;

template <>
class fast_cfile_istreambuf<char> : public std::basic_streambuf<char>
{
public:
    using char_type   = char;
    using traits_type = typename std::basic_streambuf<char_type>::traits_type;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using string_type = std::basic_string<char_type>;

    explicit fast_cfile_istreambuf(std::FILE* file)
        : file_(file)
        , last_char_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (last_char_ != 0)
            return last_char_;
        last_char_ = std::fgetc(file_);
        return last_char_;
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        last_char_ = 0;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        if (std::fgets(s, static_cast<int>(num), file_) == nullptr)
            return 0;
        return num;
    }

private:
    std::FILE* file_;
    int_type   last_char_;
};

template <>
class fast_cfile_istreambuf<wchar_t> : public std::basic_streambuf<wchar_t>
{
public:
    using char_type   = wchar_t;
    using traits_type = typename std::basic_streambuf<char_type>::traits_type;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using string_type = std::basic_string<char_type>;

    explicit fast_cfile_istreambuf(std::FILE* file)
        : file_(file)
        , last_char_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (last_char_ != 0)
            return last_char_;
        last_char_ = std::fgetwc(file_);
        return last_char_;
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        last_char_ = 0;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        if (std::fgetws(s, static_cast<int>(num), file_) == nullptr)
            return 0;
        return num;
    }

private:
    std::FILE* file_;
    int_type   last_char_;
};

//
// fast I/O stream
//

template <typename _CharTy>
class fast_basic_ostringstream : public std::basic_ostream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_basic_ostringstream()
        : std::basic_ostream<char_type>(nullptr)
        , str_()
        , buf_(str_)
    {
        this->rdbuf(&buf_);
    }

    inline const string_type& str() const
    {
        return str_;
    }

    inline void str(const string_type& s)
    {
        str_ = s;
    }

private:
    string_type                       str_;
    fast_string_ostreambuf<char_type> buf_;
};

using fast_ostringstream  = fast_basic_ostringstream<char>;
using fast_wostringstream = fast_basic_ostringstream<wchar_t>;

}  // namespace detail

}  // namespace configor
