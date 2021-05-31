// Copyright (c) 2018-2020 jsonxx - Nomango
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
#include "json_config.hpp"
#include "json_unicode.hpp"
#include "json_value.hpp"

#include <algorithm>         // std::none_of
#include <array>             // std::array
#include <initializer_list>  // std::initializer_list
#include <iomanip>           // std::fill, std::setw, std::setprecision, std::right, std::noshowbase
#include <ios>               // std::streamsize, std::hex, std::dec
#include <ostream>           // std::basic_ostream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits
#include <vector>            // std::vector

namespace jsonxx
{
//
// output_adapter
//

template <typename _CharTy>
struct output_adapter
{
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;

    virtual void write(const _CharTy ch)                     = 0;
    virtual void write(const _CharTy* str, std::size_t size) = 0;
};

template <typename _StringTy>
struct string_output_adapter : public output_adapter<typename _StringTy::value_type>
{
    using char_type   = typename _StringTy::value_type;
    using size_type   = typename _StringTy::size_type;
    using char_traits = std::char_traits<char_type>;

    string_output_adapter(_StringTy& str)
        : str_(str)
    {
    }

    virtual void write(const char_type ch) override
    {
        str_.push_back(ch);
    }

    virtual void write(const char_type* str, std::size_t size) override
    {
        str_.append(str, static_cast<size_type>(size));
    }

private:
    _StringTy& str_;
};

template <typename _CharTy>
struct stream_output_adapter : public output_adapter<_CharTy>
{
    using char_type   = _CharTy;
    using size_type   = std::streamsize;
    using char_traits = std::char_traits<char_type>;

    stream_output_adapter(std::basic_ostream<char_type>& stream)
        : stream_(stream)
    {
    }

    virtual void write(const char_type ch) override
    {
        stream_.put(ch);
    }

    virtual void write(const char_type* str, std::size_t size) override
    {
        stream_.write(str, static_cast<size_type>(size));
    }

private:
    std::basic_ostream<char_type>& stream_;
};

namespace detail
{

template <typename _CharTy>
class output_streambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = typename std::basic_streambuf<_CharTy>::char_type;
    using int_type    = typename std::basic_streambuf<_CharTy>::int_type;
    using char_traits = std::char_traits<char_type>;

    output_streambuf(output_adapter<char_type>* adapter)
        : adapter_(adapter)
    {
    }

protected:
    virtual int_type overflow(int_type c) override
    {
        if (c != EOF)
        {
            adapter_->write(char_traits::to_char_type(c));
        }
        return c;
    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize num) override
    {
        adapter_->write(s, static_cast<size_t>(num));
        return num;
    }

private:
    output_adapter<char_type>* adapter_;
};

}  // namespace detail

//
// json_serializer
//

namespace detail
{

template <typename _BasicJsonTy>
struct serializer_args
{
    using char_type  = typename _BasicJsonTy::char_type;
    using float_type = typename _BasicJsonTy::float_type;

    int       precision      = std::numeric_limits<float_type>::digits10 + 1;
    int       indent         = -1;
    char_type indent_char    = ' ';
    bool      escape_unicode = false;
};

}  // namespace detail

template <typename _BasicJsonTy>
struct json_serializer
{
    using string_type   = typename _BasicJsonTy::string_type;
    using char_type     = typename _BasicJsonTy::char_type;
    using integer_type  = typename _BasicJsonTy::integer_type;
    using float_type    = typename _BasicJsonTy::float_type;
    using boolean_type  = typename _BasicJsonTy::boolean_type;
    using array_type    = typename _BasicJsonTy::array_type;
    using object_type   = typename _BasicJsonTy::object_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;
    using args          = detail::serializer_args<_BasicJsonTy>;

    json_serializer(output_adapter<char_type>* adapter, const args& args)
        : buf_(adapter)
        , out_(&buf_)
        , args_(args)
        , indent_string_(32, args_.indent_char)
    {
        out_ << std::setprecision(args_.precision);
        out_ << std::right;
        out_ << std::noshowbase;
    }

    void dump(const _BasicJsonTy& json, const int current_indent = 0)
    {
        const bool pretty_print = (args_.indent > 0);
        switch (json.type())
        {
        case json_type::object:
        {
            auto& object = *json.value_.data.object;

            if (object.empty())
            {
                out_ << '{' << '}';
                return;
            }

            if (pretty_print)
            {
                out_ << '{' << '\n';

                const auto new_indent = static_cast<size_t>(current_indent + args_.indent);
                if (indent_string_.size() < new_indent)
                {
                    indent_string_.resize(indent_string_.size() * 2, args_.indent_char);
                }

                auto       iter = object.cbegin();
                const auto size = object.size();
                for (std::size_t i = 0; i < size; ++i, ++iter)
                {
                    out_.write(indent_string_.c_str(), new_indent);
                    out_ << '\"' << iter->first << '\"' << ':';
                    out_.write(indent_string_.c_str(), 1);
                    dump(iter->second, new_indent);

                    // not last element
                    if (i != size - 1)
                    {
                        out_ << ',' << '\n';
                    }
                }

                out_ << '\n';
                out_.write(indent_string_.c_str(), current_indent);
                out_ << '}';
            }
            else
            {
                out_ << '{';

                auto       iter = object.cbegin();
                const auto size = object.size();
                for (std::size_t i = 0; i < size; ++i, ++iter)
                {
                    out_ << '\"' << iter->first << '\"' << ':';
                    dump(iter->second, current_indent);

                    // not last element
                    if (i != size - 1)
                        out_ << ',';
                }

                out_ << '}';
            }

            return;
        }

        case json_type::array:
        {
            auto& vector = *json.value_.data.vector;

            if (vector.empty())
            {
                out_ << '[' << ']';
                return;
            }

            if (pretty_print)
            {
                out_ << '[' << '\n';

                const auto new_indent = static_cast<size_t>(current_indent + args_.indent);
                if (indent_string_.size() < new_indent)
                {
                    indent_string_.resize(indent_string_.size() * 2, args_.indent_char);
                }

                auto       iter = vector.cbegin();
                const auto size = vector.size();
                for (std::size_t i = 0; i < size; ++i, ++iter)
                {
                    out_.write(indent_string_.c_str(), new_indent);
                    dump(*iter, new_indent);

                    // not last element
                    if (i != size - 1)
                    {
                        out_ << ',' << '\n';
                    }
                }

                out_ << '\n';
                out_.write(indent_string_.c_str(), current_indent);
                out_ << ']';
            }
            else
            {
                out_ << '[';

                auto       iter = vector.cbegin();
                const auto size = vector.size();
                for (std::size_t i = 0; i < size; ++i, ++iter)
                {
                    dump(*iter, current_indent);
                    // not last element
                    if (i != size - 1)
                        out_ << ',';
                }

                out_ << ']';
            }

            return;
        }

        case json_type::string:
        {
            out_ << '\"';
            dump_string(*json.value_.data.string);
            out_ << '\"';
            return;
        }

        case json_type::boolean:
        {
            if (json.value_.data.boolean)
            {
                out_ << 't' << 'r' << 'u' << 'e';
            }
            else
            {
                out_ << 'f' << 'a' << 'l' << 's' << 'e';
            }
            return;
        }

        case json_type::number_integer:
        {
            out_ << json.value_.data.number_integer;
            return;
        }

        case json_type::number_float:
        {
            out_ << json.value_.data.number_float;
            return;
        }

        case json_type::null:
        {
            out_ << 'n' << 'u' << 'l' << 'l';
            return;
        }
        }
    }

    void dump_string(const string_type& val)
    {
        size_t   i    = 0;
        uint32_t code = 0;

        detail::unicode_reader<string_type> ur(val, args_.escape_unicode);
        while (ur.get_code(i, code))
        {
            switch (code)
            {
            case '\t':
            {
                out_ << '\\' << 't';
                break;
            }

            case '\r':
            {
                out_ << '\\' << 'r';
                break;
            }

            case '\n':
            {
                out_ << '\\' << 'n';
                break;
            }

            case '\b':
            {
                out_ << '\\' << 'b';
                break;
            }

            case '\f':
            {
                out_ << '\\' << 'f';
                break;
            }

            case '\"':
            {
                out_ << '\\' << '\"';
                break;
            }

            case '\\':
            {
                out_ << '\\' << '\\';
                break;
            }

            default:
            {
                // escape control characters
                // and non-ASCII characters (if `escape_unicode` is true)
                const bool need_escape = code <= 0x1F || (args_.escape_unicode && code >= 0x7F);
                if (!need_escape)
                {
                    // ASCII or BMP (U+0000...U+007F)
                    out_ << char_traits::to_char_type(static_cast<char_int_type>(code));
                }
                else
                {
                    if (code <= 0xFFFF)
                    {
                        // BMP: U+007F...U+FFFF
                        out_ << std::setfill(char_type('0')) << std::hex;
                        out_ << '\\' << 'u' << std::setw(4) << static_cast<uint16_t>(code);
                        out_ << std::dec;
                    }
                    else
                    {
                        // supplementary planes: U+10000...U+10FFFF
                        code = code - JSONXX_UNICODE_SUR_BASE;
                        const auto lead_surrogate =
                            static_cast<uint16_t>(JSONXX_UNICODE_SUR_LEAD_BEGIN + (code >> JSONXX_UNICODE_SUR_BITS));
                        const auto trail_surrogate =
                            static_cast<uint16_t>(JSONXX_UNICODE_SUR_TRAIL_BEGIN + (code & JSONXX_UNICODE_SUR_MAX));
                        out_ << std::setfill(char_type('0')) << std::hex;
                        out_ << '\\' << 'u' << std::setw(4) << lead_surrogate;
                        out_ << '\\' << 'u' << std::setw(4) << trail_surrogate;
                        out_ << std::dec;
                    }
                }
                break;
            }
            }
        }
    }

private:
    detail::output_streambuf<char_type> buf_;
    std::basic_ostream<char_type>       out_;
    const args&                         args_;
    string_type                         indent_string_;
};

}  // namespace jsonxx
