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
#include "json_encoding.hpp"
#include "json_stream.hpp"
#include "json_value.hpp"

#include <cstdio>            // std::snprintf
#include <initializer_list>  // std::initializer_list
#include <iomanip>           // std::fill, std::setw, std::setprecision, std::right, std::noshowbase
#include <ios>               // std::streamsize, std::hex, std::dec, std::uppercase, std::nouppercase
#include <ostream>           // std::basic_ostream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits
#include <utility>           // std::forward

namespace jsonxx
{

namespace detail
{

//
// serialize functions
//

struct snprintf_t
{
    template <typename _CharTy, typename _IntTy>
    static inline std::basic_ostream<_CharTy>& one_integer(std::basic_ostream<_CharTy>& os, const _IntTy val)
    {
        char buffer[32] = {};
        internal_snprintf(os, buffer, sizeof(buffer), "%d", val);
        return os;
    }

    template <typename _CharTy, typename _IntTy>
    static inline std::basic_ostream<_CharTy>& one_hex(std::basic_ostream<_CharTy>& os, const _IntTy val)
    {
        char buffer[7] = {};
        internal_snprintf(os, buffer, sizeof(buffer), "\\u%04X", val);
        return os;
    }

    template <typename _CharTy, typename _FloatTy>
    static inline std::basic_ostream<_CharTy>& one_float(std::basic_ostream<_CharTy>& os, const _FloatTy val)
    {
        char buffer[32] = {};
        internal_snprintf(os, buffer, sizeof(buffer), "%.*g", static_cast<int>(os.precision()), val);
        return os;
    }

private:
    template <typename _CharTy, typename... _Args>
    static inline void internal_snprintf(std::basic_ostream<_CharTy>& os, char* buffer, size_t size, const char* format,
                                         _Args&&... args)
    {
        const auto len = std::snprintf(buffer, size, format, std::forward<_Args>(args)...);
        if (len > 0)
            copy_simple_str_to_big(os, buffer, static_cast<size_t>(len));
        else
            os.setstate(std::ios_base::failbit);
    }

    template <typename _CharTy>
    static inline void copy_simple_str_to_big(std::basic_ostream<_CharTy>& os, const char* str, size_t len)
    {
        for (size_t i = 0; i < len; i++)
            os.put(static_cast<_CharTy>(str[i]));
    }
};

template <typename _IntTy>
struct serializable_integer
{
    const _IntTy i;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os,
                                                          const serializable_integer&  i)
    {
        return os << i.i;
    }

    friend std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const serializable_integer& i)
    {
        return snprintf_t::one_integer(os, i.i);
    }

    friend std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const serializable_integer& i)
    {
        return snprintf_t::one_integer(os, i.i);
    }
};

template <typename _IntTy>
struct serializable_hex
{
    const _IntTy i;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os, const serializable_hex& i)
    {
        os << std::setfill(_CharTy('0')) << std::hex << std::uppercase;
        os << '\\' << 'u' << std::setw(4) << i.i;
        os << std::dec << std::nouppercase;
        return os;
    }

    friend std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const serializable_hex& i)
    {
        return snprintf_t::one_hex(os, i.i);
    }

    friend std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const serializable_hex& i)
    {
        return snprintf_t::one_hex(os, i.i);
    }
};

template <typename _FloatTy>
struct serializable_float
{
    const _FloatTy f;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os, const serializable_float& f)
    {
        return os << f.f;
    }

    friend std::basic_ostream<char16_t>& operator<<(std::basic_ostream<char16_t>& os, const serializable_float& f)
    {
        return snprintf_t::one_float(os, f.f);
    }

    friend std::basic_ostream<char32_t>& operator<<(std::basic_ostream<char32_t>& os, const serializable_float& f)
    {
        return snprintf_t::one_float(os, f.f);
    }
};

namespace
{

template <typename _IntTy>
inline serializable_integer<_IntTy> serialize_integer(const _IntTy i)
{
    return serializable_integer<_IntTy>{ i };
}

template <typename _IntTy>
inline serializable_hex<_IntTy> serialize_hex(const _IntTy i)
{
    return serializable_hex<_IntTy>{ i };
}

template <typename _FloatTy>
inline serializable_float<_FloatTy> serialize_float(const _FloatTy f)
{
    return serializable_float<_FloatTy>{ f };
}

}  // namespace

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
        if (i.indent_char_)
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

template <typename _BasicJsonTy>
struct serializer_args
{
    using char_type  = typename _BasicJsonTy::char_type;
    using float_type = typename _BasicJsonTy::float_type;

    int          precision      = std::numeric_limits<float_type>::digits10 + 1;
    unsigned int indent         = 0;
    char_type    indent_char    = ' ';
    bool         escape_unicode = false;
};

//
// json_serializer
//

template <typename _BasicJsonTy, template <class _CharTy> class _Encoding>
struct json_serializer
{
    using char_type     = typename _BasicJsonTy::char_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;
    using string_type   = typename _BasicJsonTy::string_type;
    using integer_type  = typename _BasicJsonTy::integer_type;
    using float_type    = typename _BasicJsonTy::float_type;
    using encoding      = _Encoding<char_type>;
    using args          = serializer_args<_BasicJsonTy>;

    json_serializer(std::basic_ostream<char_type>& os, const args& args)
        : os_(os)
        , args_(args)
        , indent_()
    {
        os_ << std::setprecision(args_.precision);
        os_ << std::right;
        os_ << std::noshowbase;

        const bool pretty_print = (args_.indent > 0);
        if (pretty_print)
        {
            indent_.init(args_.indent_char);
        }
    }

    void dump(const _BasicJsonTy& json, const unsigned int current_indent = 0)
    {
        switch (json.type())
        {
        case json_type::object:
        {
            const auto& object = *json.value_.data.object;

            if (object.empty())
            {
                output({ '{', '}' });
                return;
            }

            output('{');
            output_newline();

            auto       iter         = object.cbegin();
            const auto size         = object.size();
            const auto elem_indents = current_indent + args_.indent;
            for (std::size_t i = 0; i < size; ++i, ++iter)
            {
                output_indent(elem_indents);
                output('\"');
                dump_string(iter->first);
                output({ '\"', ':' });
                output_indent(1);
                dump(iter->second, elem_indents);

                // not last element
                if (i != size - 1)
                {
                    output(',');
                    output_newline();
                }
            }
            output_newline();
            output_indent(current_indent);
            output('}');
            return;
        }

        case json_type::array:
        {
            auto& v = *json.value_.data.vector;

            if (v.empty())
            {
                output({ '[', ']' });
                return;
            }

            output('[');
            output_newline();

            const auto elem_indents = current_indent + args_.indent;
            const auto size         = v.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                output_indent(elem_indents);
                dump(v.at(i), elem_indents);

                // not last element
                if (i != size - 1)
                {
                    output(',');
                    output_newline();
                }
            }
            output_newline();
            output_indent(current_indent);
            output(']');
            return;
        }

        case json_type::string:
        {
            output('\"');
            dump_string(*json.value_.data.string);
            output('\"');
            return;
        }

        case json_type::boolean:
        {
            if (json.value_.data.boolean)
            {
                output({ 't', 'r', 'u', 'e' });
            }
            else
            {
                output({ 'f', 'a', 'l', 's', 'e' });
            }
            return;
        }

        case json_type::number_integer:
        {
            os_ << detail::serialize_integer(json.value_.data.number_integer);
            return;
        }

        case json_type::number_float:
        {
            os_ << detail::serialize_float(json.value_.data.number_float);
            return;
        }

        case json_type::null:
        {
            output({ 'n', 'u', 'l', 'l' });
            return;
        }
        }
    }

    void dump_string(const string_type& val)
    {
        detail::fast_string_istreambuf<char_type> buf{ val };
        std::basic_istream<char_type>             iss{ &buf };

        uint32_t code = 0;
        while (encoding::decode(iss, code))
        {
            if (!iss.good())
            {
                throw json_serialize_error("unexpected character");
            }

            switch (code)
            {
            case '\t':
            {
                output({ '\\', 't' });
                break;
            }

            case '\r':
            {
                output({ '\\', 'r' });
                break;
            }

            case '\n':
            {
                output({ '\\', 'n' });
                break;
            }

            case '\b':
            {
                output({ '\\', 'b' });
                break;
            }

            case '\f':
            {
                output({ '\\', 'f' });
                break;
            }

            case '\"':
            {
                output({ '\\', '\"' });
                break;
            }

            case '\\':
            {
                output({ '\\', '\\' });
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
                    encoding::encode(os_, code);
                    if (!os_.good())
                    {
                        throw json_serialize_error("unexpected encoding error");
                    }
                }
                else
                {
                    if (code <= 0xFFFF)
                    {
                        // BMP: U+007F...U+FFFF
                        os_ << detail::serialize_hex(static_cast<uint16_t>(code));
                    }
                    else
                    {
                        // supplementary planes: U+10000...U+10FFFF
                        uint32_t lead_surrogate = 0, trail_surrogate = 0;
                        detail::separate_surrogates(code, lead_surrogate, trail_surrogate);

                        os_ << detail::serialize_hex(lead_surrogate);
                        os_ << detail::serialize_hex(trail_surrogate);
                    }
                }
                break;
            }
            }
        }
    }

private:
    void output(char_type ch)
    {
        os_.put(ch);
    }

    void output(std::initializer_list<char_type> list)
    {
        os_.write(list.begin(), static_cast<std::streamsize>(list.size()));
    }

    void output_indent(unsigned int size)
    {
        os_ << indent_ * size;
    }

    void output_newline()
    {
        if (args_.indent > 0 /* pretty print */)
            os_.put('\n');
    }

private:
    const args& args_;

    std::basic_ostream<char_type>& os_;
    detail::indent<string_type>    indent_;
};

}  // namespace jsonxx
