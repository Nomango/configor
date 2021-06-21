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

#include <algorithm>         // std::none_of
#include <array>             // std::array
#include <initializer_list>  // std::initializer_list
#include <iomanip>           // std::fill, std::setw, std::setprecision, std::right, std::noshowbase
#include <ios>               // std::streamsize, std::hex, std::dec, std::uppercase, std::nouppercase
#include <ostream>           // std::basic_ostream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits
#include <vector>            // std::vector

namespace jsonxx
{

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
    using encoding      = _Encoding<char_type>;
    using args          = serializer_args<_BasicJsonTy>;

    json_serializer(std::basic_ostream<char_type>& os, const args& args)
        : os_(os)
        , args_(args)
        , indent_()
        , newline_(json_serializer::stream_do_nothing)
    {
        os_ << std::setprecision(args_.precision);
        os_ << std::right;
        os_ << std::noshowbase;

        const bool pretty_print = (args_.indent > 0);
        if (pretty_print)
        {
            newline_ = json_serializer::stream_new_line;
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
                os_ << '{' << '}';
                return;
            }

            os_ << '{' << newline_;

            auto       iter         = object.cbegin();
            const auto size         = object.size();
            const auto elem_indents = current_indent + args_.indent;
            for (std::size_t i = 0; i < size; ++i, ++iter)
            {
                os_ << indent_ * elem_indents << '\"';
                dump_string(iter->first);
                os_ << '\"' << ':';
                os_ << indent_ * 1;
                dump(iter->second, elem_indents);

                // not last element
                if (i != size - 1)
                {
                    os_ << ',' << newline_;
                }
            }
            os_ << newline_ << indent_ * current_indent << '}';
            return;
        }

        case json_type::array:
        {
            auto& v = *json.value_.data.vector;

            if (v.empty())
            {
                os_ << '[' << ']';
                return;
            }

            os_ << '[' << newline_;

            const auto elem_indents = current_indent + args_.indent;
            const auto size         = v.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                os_ << indent_ * elem_indents;
                dump(v.at(i), elem_indents);

                // not last element
                if (i != size - 1)
                {
                    os_ << ',' << newline_;
                }
            }
            os_ << newline_ << indent_ * current_indent << ']';
            return;
        }

        case json_type::string:
        {
            os_ << '\"';
            dump_string(*json.value_.data.string);
            os_ << '\"';
            return;
        }

        case json_type::boolean:
        {
            if (json.value_.data.boolean)
            {
                os_ << 't' << 'r' << 'u' << 'e';
            }
            else
            {
                os_ << 'f' << 'a' << 'l' << 's' << 'e';
            }
            return;
        }

        case json_type::number_integer:
        {
            os_ << json.value_.data.number_integer;
            return;
        }

        case json_type::number_float:
        {
            os_ << json.value_.data.number_float;
            return;
        }

        case json_type::null:
        {
            os_ << 'n' << 'u' << 'l' << 'l';
            return;
        }
        }
    }

    void dump_string(const string_type& val)
    {
        detail::fast_string_istreambuf<char_type> buf{ val };
        std::basic_istream<char_type> iss{ &buf };

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
                os_ << '\\' << 't';
                break;
            }

            case '\r':
            {
                os_ << '\\' << 'r';
                break;
            }

            case '\n':
            {
                os_ << '\\' << 'n';
                break;
            }

            case '\b':
            {
                os_ << '\\' << 'b';
                break;
            }

            case '\f':
            {
                os_ << '\\' << 'f';
                break;
            }

            case '\"':
            {
                os_ << '\\' << '\"';
                break;
            }

            case '\\':
            {
                os_ << '\\' << '\\';
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
                        os_ << std::setfill(char_type('0')) << std::hex << std::uppercase;
                        os_ << '\\' << 'u' << std::setw(4) << static_cast<uint16_t>(code);
                        os_ << std::dec << std::nouppercase;
                    }
                    else
                    {
                        // supplementary planes: U+10000...U+10FFFF
                        uint32_t lead_surrogate = 0, trail_surrogate = 0;
                        detail::separate_surrogates(code, lead_surrogate, trail_surrogate);

                        os_ << std::setfill(char_type('0')) << std::hex << std::uppercase;
                        os_ << '\\' << 'u' << std::setw(4) << lead_surrogate;
                        os_ << '\\' << 'u' << std::setw(4) << trail_surrogate;
                        os_ << std::dec << std::nouppercase;
                    }
                }
                break;
            }
            }
        }
    }

private:
    class indent
    {
    public:
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

    static inline std::basic_ostream<char_type>& stream_do_nothing(std::basic_ostream<char_type>& out)
    {
        return out;
    }

    static inline std::basic_ostream<char_type>& stream_new_line(std::basic_ostream<char_type>& out)
    {
        return out << '\n';
    }

private:
    std::basic_ostream<char_type>& os_;

    const args& args_;
    indent      indent_;

    using stream_func = std::basic_ostream<char_type>& (*)(std::basic_ostream<char_type>&);
    stream_func newline_;
};

}  // namespace jsonxx
