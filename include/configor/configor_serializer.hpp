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
#include "configor_encoding.hpp"
#include "configor_stream.hpp"
#include "configor_token.hpp"
#include "configor_value.hpp"

#include <ios>          // std::streamsize
#include <ostream>      // std::basic_ostream
#include <type_traits>  // std::char_traits

namespace configor
{

template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class basic_serializer
{
public:
    using config_type  = _ConfTy;
    using integer_type = typename _ConfTy::integer_type;
    using float_type   = typename _ConfTy::float_type;
    using char_type    = typename _ConfTy::char_type;
    using string_type  = typename _ConfTy::string_type;

    virtual void target(std::basic_ostream<char_type>& os) = 0;

    virtual void next(token_type t) = 0;

    virtual void put_integer(integer_type i)      = 0;
    virtual void put_float(float_type f)          = 0;
    virtual void put_string(const string_type& s) = 0;
};

namespace detail
{
template <typename _SerialTy, typename _ConfTy = typename _SerialTy::config_type>
void do_dump_config(const _ConfTy& c, _SerialTy& serializer);
}

template <typename _SerialTy, typename _ConfTy = typename _SerialTy::config_type,
          typename = typename std::enable_if<is_config<_ConfTy>::value>::type>
void dump_config(const _ConfTy& c, std::basic_ostream<typename _ConfTy::char_type>& os, _SerialTy& serializer,
                 error_handler* eh)
{
    using char_type = typename _ConfTy::char_type;

    try
    {
        serializer.target(os);
        detail::do_dump_config(c, serializer);
        serializer.next(token_type::end_of_input);
    }
    catch (...)
    {
        if (eh)
            eh->handle(std::current_exception());
        else
            throw;
    }
}

template <typename _SerialTy, typename _ConfTy = typename _SerialTy::config_type,
          typename = typename std::enable_if<is_config<_ConfTy>::value
                                             && std::is_default_constructible<_SerialTy>::value>::type>
void dump_config(const _ConfTy& c, std::basic_ostream<typename _ConfTy::char_type>& os, error_handler* eh = nullptr)
{
    _SerialTy s{};
    dump_config(c, os, s, eh);
}

namespace detail
{

template <typename _SerialTy, typename _ConfTy>
void do_dump_config(const _ConfTy& c, _SerialTy& serializer)
{
    switch (c.type())
    {
    case config_value_type::object:
    {
        const auto& object = *c.raw_value().data.object;

        serializer.next(token_type::begin_object);
        if (object.empty())
        {
            serializer.next(token_type::end_object);
            return;
        }

        auto iter = object.cbegin();
        auto size = object.size();
        for (std::size_t i = 0; i < size; ++i, ++iter)
        {
            serializer.next(token_type::value_string);
            serializer.put_string(iter->first);
            serializer.next(token_type::name_separator);

            do_dump_config(iter->second, serializer);

            // not last element
            if (i != size - 1)
            {
                serializer.next(token_type::value_separator);
            }
        }
        serializer.next(token_type::end_object);
        return;
    }

    case config_value_type::array:
    {
        serializer.next(token_type::begin_array);

        auto& v = *c.raw_value().data.vector;
        if (v.empty())
        {
            serializer.next(token_type::end_array);
            return;
        }

        const auto size = v.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            do_dump_config(v.at(i), serializer);
            // not last element
            if (i != size - 1)
            {
                serializer.next(token_type::value_separator);
            }
        }
        serializer.next(token_type::end_array);
        return;
    }

    case config_value_type::string:
    {
        serializer.next(token_type::value_string);
        serializer.put_string(*c.raw_value().data.string);
        return;
    }

    case config_value_type::boolean:
    {
        if (c.raw_value().data.boolean)
        {
            serializer.next(token_type::literal_true);
        }
        else
        {
            serializer.next(token_type::literal_false);
        }
        return;
    }

    case config_value_type::number_integer:
    {
        serializer.next(token_type::value_integer);
        serializer.put_integer(c.raw_value().data.number_integer);
        return;
    }

    case config_value_type::number_float:
    {
        serializer.next(token_type::value_float);
        serializer.put_float(c.raw_value().data.number_float);
        return;
    }

    case config_value_type::null:
    {
        serializer.next(token_type::literal_null);
        return;
    }
    }
}

//
// indent
//

template <typename _StrTy>
class indent
{
public:
    using string_type = _StrTy;
    using char_type   = typename string_type::value_type;

    inline indent()
        : length_(0)
        , indent_char_(0)
        , indent_string_()
    {
    }

    inline void init(char_type ch)
    {
        indent_char_ = ch;
        indent_string_.resize(16, indent_char_);
    }

    inline indent& operator*(unsigned int length)
    {
        if (indent_char_)
        {
            length_ = length;
            if (indent_string_.size() < static_cast<size_t>(length))
            {
                indent_string_.resize(indent_string_.size() * 2, indent_char_);
            }
        }
        return *this;
    }

    friend inline std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const indent& i)
    {
        if (i.indent_char_ && i.length_)
        {
            os.write(i.indent_string_.c_str(), static_cast<std::streamsize>(i.length_));
        }
        return os;
    }

private:
    unsigned int length_;
    char_type    indent_char_;
    string_type  indent_string_;
};

}  // namespace detail

}  // namespace configor
