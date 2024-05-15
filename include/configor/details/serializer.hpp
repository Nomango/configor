// Copyright (c) 2018-2020 configor - Nomango
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
#include "encoding.hpp"
#include "stream.hpp"
#include "token.hpp"
#include "value.hpp"

#include <functional>        // std::function
#include <initializer_list>  // std::initializer_list
#include <ios>               // std::streamsize
#include <locale>            // std::locale
#include <ostream>           // std::basic_ostream

namespace configor
{

namespace detail
{

template <typename _ValTy, typename _TargetCharTy>
class basic_serializer
{
public:
    using value_type       = _ValTy;
    using source_char_type = typename value_type::char_type;
    using target_char_type = _TargetCharTy;

    explicit basic_serializer(std::basic_ostream<target_char_type>& os)
        : os_(os.rdbuf())
        , err_handler_(nullptr)
        , source_decoder_(nullptr)
        , target_encoder_(nullptr)
    {
        os_.setf(os.flags(), std::ios_base::floatfield);
        os_.imbue(std::locale(std::locale::classic(), os.getloc(), std::locale::collate | std::locale::ctype));
    }

    virtual void dump(const value_type& c)
    {
        try
        {
            do_dump(c);
            next(token_type::end_of_input);
        }
        catch (...)
        {
            if (err_handler_)
                err_handler_->handle(std::current_exception());
            else
                throw;
        }
    }

    inline void set_error_handler(configor::error_handler* eh)
    {
        err_handler_ = eh;
    }

    template <template <class> class _Encoding>
    inline void set_source_encoding()
    {
        source_decoder_ = _Encoding<source_char_type>::decode;
    }

    template <template <class> class _Encoding>
    inline void set_target_encoding()
    {
        target_encoder_ = _Encoding<target_char_type>::encode;
    }

protected:
    virtual void next(token_type t) = 0;

    virtual void put_integer(typename value_type::integer_type i)      = 0;
    virtual void put_float(typename value_type::float_type f)          = 0;
    virtual void put_string(const typename value_type::string_type& s) = 0;

    virtual void do_dump(const value_type& c)
    {
        switch (c.type())
        {
        case value_constant::object:
        {
            const auto& object = *c.data().object;

            next(token_type::begin_object);
            if (object.empty())
            {
                next(token_type::end_object);
                return;
            }

            auto iter = object.cbegin();
            auto size = object.size();
            for (std::size_t i = 0; i < size; ++i, ++iter)
            {
                next(token_type::value_string);
                put_string(iter->first);
                next(token_type::name_separator);

                do_dump(iter->second);

                // not last element
                if (i != size - 1)
                {
                    next(token_type::value_separator);
                }
            }
            next(token_type::end_object);
            return;
        }

        case value_constant::array:
        {
            next(token_type::begin_array);

            auto& v = *c.data().vector;
            if (v.empty())
            {
                next(token_type::end_array);
                return;
            }

            const auto size = v.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                do_dump(v.at(i));
                // not last element
                if (i != size - 1)
                {
                    next(token_type::value_separator);
                }
            }
            next(token_type::end_array);
            return;
        }

        case value_constant::string:
        {
            next(token_type::value_string);
            put_string(*c.data().string);
            return;
        }

        case value_constant::boolean:
        {
            if (c.data().boolean)
            {
                next(token_type::literal_true);
            }
            else
            {
                next(token_type::literal_false);
            }
            return;
        }

        case value_constant::integer:
        {
            next(token_type::value_integer);
            put_integer(c.data().integer);
            return;
        }

        case value_constant::floating:
        {
            next(token_type::value_float);
            put_float(c.data().floating);
            return;
        }

        case value_constant::null:
        {
            next(token_type::literal_null);
            return;
        }
        }
    }

protected:
    std::basic_ostream<target_char_type> os_;
    error_handler*                       err_handler_;
    encoding::decoder<source_char_type>  source_decoder_;
    encoding::encoder<target_char_type>  target_encoder_;
};

//
// serializable
//

template <class _Args, template <class, class> class _SerializerTy, template <class> class _DefaultEncoding>
class serializable
{
public:
    using value_type = basic_value<_Args>;

    template <typename _TargetCharTy>
    using serializer_type = _SerializerTy<value_type, _TargetCharTy>;

    template <typename _TargetCharTy>
    using serializer_option = typename serializer_type<_TargetCharTy>::option;

    // dump to stream
    template <typename _TargetCharTy>
    static void dump(std::basic_ostream<_TargetCharTy>& os, const value_type& v,
                     std::initializer_list<serializer_option<_TargetCharTy>> options = {})
    {
        serializer_type<_TargetCharTy> s{ os };
        s.template set_source_encoding<_DefaultEncoding>();
        s.template set_target_encoding<_DefaultEncoding>();
        s.prepare(options);
        s.dump(v);
    }

    // dump to string
    template <typename _TargetCharTy>
    static void dump(typename _Args::template string_type<_TargetCharTy>& str, const value_type& v,
                     std::initializer_list<serializer_option<_TargetCharTy>> options = {})
    {
        detail::fast_string_ostreambuf<_TargetCharTy> buf{ str };
        std::basic_ostream<_TargetCharTy>             os{ &buf };
        return dump<_TargetCharTy>(os, v, options);
    }

    template <typename _TargetCharTy = typename value_type::char_type>
    static typename _Args::template string_type<_TargetCharTy>
    dump(const value_type& v, std::initializer_list<serializer_option<_TargetCharTy>> options = {})
    {
        typename _Args::template string_type<_TargetCharTy> result;
        dump<_TargetCharTy>(result, v, options);
        return result;
    }
};

//
// indent
//

template <typename _CharTy>
class indent
{
public:
    using char_type   = _CharTy;
    using string_type = std::basic_string<char_type>;

    indent(uint8_t step, char_type ch)
        : step_(step)
        , depth_(0)
        , indent_char_(ch)
        , indent_string_()
    {
        reverse(static_cast<size_t>(step_ * 2));
    }

    inline void operator++()
    {
        ++depth_;
        reverse(static_cast<size_t>(depth_ * step_));
    }

    inline void operator--()
    {
        --depth_;
    }

    inline void put(std::basic_ostream<char_type>& os) const
    {
        os.write(indent_string_.c_str(), static_cast<std::streamsize>(depth_ * step_));
    }

    inline void put(std::basic_ostream<char_type>& os, int length)
    {
        reverse(static_cast<size_t>(length));
        os.write(indent_string_.c_str(), static_cast<std::streamsize>(length));
    }

    friend inline std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const indent& i)
    {
        if (i.indent_char_ && i.step_ > 0 && i.depth_ > 0)
        {
            i.put(os);
        }
        return os;
    }

private:
    void reverse(size_t length)
    {
        if (indent_char_)
        {
            if (indent_string_.size() < length)
            {
                indent_string_.resize(length + static_cast<size_t>(step_ * 2), indent_char_);
            }
        }
    }

private:
    uint8_t     step_;
    uint16_t    depth_;
    char_type   indent_char_;
    string_type indent_string_;
};

}  // namespace detail

}  // namespace configor
