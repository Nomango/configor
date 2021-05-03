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
#include <cstdio>  // snprintf
#include <cwchar>  // swprintf
#include <cassert>  // assert
#include <type_traits>  // std::char_traits
#include <ios>  // std::basic_ostream
#include <array>  // std::array
#include <algorithm>  // std::none_of
#include <initializer_list>  // std::initializer_list
#include "json_exception.hpp"

namespace jsonxx
{
    //
    // output_adapter
    //

    template <typename _CharTy>
    struct output_adapter
    {
        using char_type = _CharTy;
        using char_traits = std::char_traits<char_type>;

        virtual void write(const _CharTy ch) = 0;
        virtual void write(const _CharTy *str, std::size_t size) = 0;
    };

    template <typename _StringTy>
    struct string_output_adapter
        : public output_adapter<typename _StringTy::value_type>
    {
        using char_type = typename _StringTy::value_type;
        using size_type = typename _StringTy::size_type;
        using char_traits = std::char_traits<char_type>;

        string_output_adapter(_StringTy &str) : str_(str) {}

        virtual void write(const char_type ch) override
        {
            str_.push_back(ch);
        }

        virtual void write(const char_type *str, std::size_t size) override
        {
            str_.append(str, static_cast<size_type>(size));
        }

    private:
        _StringTy &str_;
    };

    template <typename _CharTy>
    struct stream_output_adapter
        : public output_adapter<_CharTy>
    {
        using char_type = _CharTy;
        using size_type = typename std::streamsize;
        using char_traits = std::char_traits<char_type>;

        stream_output_adapter(std::basic_ostream<char_type> &stream) : stream_(stream) {}

        virtual void write(const char_type ch) override
        {
            stream_.put(ch);
        }

        virtual void write(const char_type *str, std::size_t size) override
        {
            stream_.write(str, static_cast<size_type>(size));
        }

    private:
        std::basic_ostream<char_type> &stream_;
    };

    //
    // json_serializer
    //

    namespace detail
    {
        template <typename _CharTy>
        struct snprintf_t
        {
        };

        template <>
        struct snprintf_t<char>
        {
            using char_type = char;

            template <typename _FloatTy>
            static inline size_t one_float(char_type* str, size_t size, _FloatTy val)
            {
                static constexpr auto digits = std::numeric_limits<_FloatTy>::max_digits10;
                return internal_snprintf(str, size, "%.*g", digits, val);
            }

            static inline size_t one_unicode(char_type* str, uint16_t code)
            {
                return internal_snprintf(str, 7, "\\u%04X", code);
            }

            static inline size_t two_unicodes(char_type* str, uint16_t code1, uint16_t code2)
            {
                return internal_snprintf(str, 13, "\\u%04X\\u%04X", code1, code2);
            }

            template <typename... _Args>
            static inline size_t internal_snprintf(char_type* str, size_t size, const char_type* format, _Args&&... args)
            {
                const auto len = std::snprintf(str, size, format, std::forward<_Args>(args)...);
                // check len
                if (len < 0)
                {
                    throw json_serialize_error("snprintf failed");
                }
                return static_cast<size_t>(len);
            }
        };

        template <>
        struct snprintf_t<wchar_t>
        {
            using char_type = wchar_t;

            template <typename _FloatTy>
            static inline size_t one_float(char_type* str, size_t size, _FloatTy val)
            {
                static constexpr auto digits = std::numeric_limits<_FloatTy>::max_digits10;
                return internal_snprintf(str, size, L"%.*g", digits, val);
            }

            static inline size_t one_unicode(char_type* str, uint16_t code)
            {
                return internal_snprintf(str, 7, L"\\u%04X", code);
            }

            static inline size_t two_unicodes(char_type* str, uint16_t code1, uint16_t code2)
            {
                return internal_snprintf(str, 13, L"\\u%04X\\u%04X", code1, code2);
            }

            template <typename... _Args>
            static inline size_t internal_snprintf(char_type* str, size_t size, const char_type* format, _Args&&... args)
            {
                const auto len = std::swprintf(str, size, format, std::forward<_Args>(args)...);
                // check len
                if (len < 0)
                {
                    throw json_serialize_error("snprintf failed");
                }
                return static_cast<size_t>(len);
            }
        };

        template <typename _StrTy>
        struct unicode_serializer;

        template <>
        struct unicode_serializer<std::string>
        {
            using string_type = std::string;

            const string_type &val;
            const bool escape_utf8;
            uint8_t state = 0;

            unicode_serializer(const string_type &val, const bool escape_utf8)
                : val(val), escape_utf8(escape_utf8)
            {
            }

            bool read_code(size_t& i, uint32_t& code)
            {
                static const uint8_t STATE_ACCEPT = 0;
                static const uint8_t STATE_REJECT = 0;
                static const std::array<std::uint8_t, 400> utf8d =
                {
                    {
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 00..1F
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20..3F
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..5F
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..7F
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // 80..9F
                        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // A0..BF
                        8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C0..DF
                        0xA, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x3, // E0..EF
                        0xB, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // F0..FF
                        0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, 0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, // s1..s2
                        1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, // s3..s4
                        1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, // s5..s6
                        1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 // s7..s8
                    }
                };

                if (i >= val.size())
                {
                    if (state != STATE_ACCEPT)
                    {
                        throw json_serialize_error("string was incomplete");
                    }
                    return false;
                }

                // read one byte
                const auto byte = static_cast<uint8_t>(val.at(i));
                i++;

                if (!escape_utf8)
                {
                    // code point will not be escaped
                    code = static_cast<uint32_t>(byte);
                    return true;
                }

                assert(static_cast<size_t>(byte) < utf8d.size());
                const std::uint8_t type = utf8d[byte];

                if (state == STATE_ACCEPT)
                {
                    code = static_cast<uint32_t>((0xFFu >> type) & (byte));
                }
                else
                {
                    code = static_cast<uint32_t>((byte & 0x3fu) | (code << 6u));
                }

                std::size_t index = 256u + static_cast<size_t>(state) * 16u + static_cast<size_t>(type);
                assert(index < utf8d.size());
                state = utf8d[index];

                if (state == STATE_ACCEPT)
                {
                    return true;
                }
                else if (state == STATE_REJECT)
                {
                    throw json_serialize_error("unexpected utf-8 character");
                }
                else
                {
                    // found yet incomplete multi-byte code point
                    return this->read_code(i, code);
                }
                return true;
            }
        };

        template <>
        struct unicode_serializer<std::wstring>
        {
            using string_type = std::wstring;

            const string_type &val;

            unicode_serializer(const string_type &val, const bool escape_utf8)
                : val(val)
            {
            }

            bool read_code(size_t& i, uint32_t& code)
            {
                if (i >= val.size())
                {
                    return false;
                }
                code = static_cast<uint32_t>(val.at(i));
                i++;
                return true;
            }
        };
    }

    template <typename _BasicJsonTy>
    struct json_serializer
    {
        using string_type = typename _BasicJsonTy::string_type;
        using char_type = typename _BasicJsonTy::char_type;
        using integer_type = typename _BasicJsonTy::integer_type;
        using float_type = typename _BasicJsonTy::float_type;
        using boolean_type = typename _BasicJsonTy::boolean_type;
        using array_type = typename _BasicJsonTy::array_type;
        using object_type = typename _BasicJsonTy::object_type;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        json_serializer(output_adapter<char_type> *out, const char_type indent_char)
            : out(out), indent_char(indent_char), indent_string(32, indent_char)
        {
        }

        void dump(
            const _BasicJsonTy &json,
            const bool pretty_print,
            const bool escape_utf8,
            const unsigned int indent_step,
            const unsigned int current_indent = 0)
        {
            switch (json.type())
            {
            case json_type::object:
            {
                auto &object = *json.value_.data.object;

                if (object.empty())
                {
                    output({'{', '}'});
                    return;
                }

                if (pretty_print)
                {
                    output({'{', '\n'});

                    const auto new_indent = current_indent + indent_step;
                    if (indent_string.size() < new_indent)
                    {
                        indent_string.resize(indent_string.size() * 2, indent_char);
                    }

                    auto iter = object.cbegin();
                    const auto size = object.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        output(indent_string.c_str(), new_indent);
                        output('\"');
                        output(iter->first.c_str());
                        output({'\"', ':'});
                        output(indent_string.c_str(), 1);
                        dump(iter->second, pretty_print, escape_utf8, indent_step, new_indent);

                        // not last element
                        if (i != size - 1)
                        {
                            output({',', '\n'});
                        }
                    }

                    output('\n');
                    output(indent_string.c_str(), current_indent);
                    output('}');
                }
                else
                {
                    output('{');

                    auto iter = object.cbegin();
                    const auto size = object.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        output('\"');
                        output(iter->first.c_str());
                        output({'\"', ':'});
                        dump(iter->second, pretty_print, escape_utf8, indent_step, current_indent);

                        // not last element
                        if (i != size - 1)
                            output(',');
                    }

                    output('}');
                }

                return;
            }

            case json_type::array:
            {
                auto &vector = *json.value_.data.vector;

                if (vector.empty())
                {
                    output({'[', ']'});
                    return;
                }

                if (pretty_print)
                {
                    output({'[', '\n'});

                    const auto new_indent = current_indent + indent_step;
                    if (indent_string.size() < new_indent)
                    {
                        indent_string.resize(indent_string.size() * 2, indent_char);
                    }

                    auto iter = vector.cbegin();
                    const auto size = vector.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        output(indent_string.c_str(), new_indent);
                        dump(*iter, pretty_print, escape_utf8, indent_step, new_indent);

                        // not last element
                        if (i != size - 1)
                        {
                            output({',', '\n'});
                        }
                    }

                    output('\n');
                    output(indent_string.c_str(), current_indent);
                    output(']');
                }
                else
                {
                    output('[');

                    auto iter = vector.cbegin();
                    const auto size = vector.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        dump(*iter, pretty_print, escape_utf8, indent_step, current_indent);
                        // not last element
                        if (i != size - 1)
                            output(',');
                    }

                    output(']');
                }

                return;
            }

            case json_type::string:
            {
                output('\"');
                dump_string(*json.value_.data.string, escape_utf8);
                output('\"');
                return;
            }

            case json_type::boolean:
            {
                if (json.value_.data.boolean)
                {
                    output({'t', 'r', 'u', 'e'});
                }
                else
                {
                    output({'f', 'a', 'l', 's', 'e'});
                }
                return;
            }

            case json_type::number_integer:
            {
                dump_integer(json.value_.data.number_integer);
                return;
            }

            case json_type::number_float:
            {
                dump_float(json.value_.data.number_float);
                return;
            }

            case json_type::null:
            {
                output({'n', 'u', 'l', 'l'});
                return;
            }
            }
        }

        void dump_integer(integer_type val)
        {
            if (val == 0)
            {
                output('0');
                return;
            }

            auto uval = static_cast<typename std::make_unsigned<integer_type>::type>(val);

            if (val < 0)
                uval = 0 - uval;

            auto next = number_buffer.rbegin();
            *next = '\0';

            do
            {
                *(++next) = static_cast<char_type>('0' + uval % 10);
                uval /= 10;
            } while (uval != 0);

            if (val < 0)
                *(++next) = '-';

            output(&(*next));
        }

        void dump_float(float_type val)
        {
            const auto len = detail::snprintf_t<char_type>::one_float(number_buffer.data(), number_buffer.size(), val);
            assert(number_buffer.size() > len);

            output(number_buffer.data(), len);

            // determine if need to append ".0"
            if (std::none_of(number_buffer.begin(), number_buffer.begin() + len + 1, [](char_type c) { return c == '.'; }))
            {
                output({'.', '0'});
            }
        }

        void dump_string(const string_type &val, const bool escape_utf8)
        {
            size_t i = 0;
            uint32_t code = 0;
            detail::unicode_serializer<string_type> us(val, escape_utf8);
            while (us.read_code(i, code))
            {
                switch (code)
                {
                case '\t':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = 't';
                    buffer_idx += 2;
                    break;
                }

                case '\r':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = 'r';
                    buffer_idx += 2;
                    break;
                }

                case '\n':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = 'n';
                    buffer_idx += 2;
                    break;
                }

                case '\b':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = 'b';
                    buffer_idx += 2;
                    break;
                }

                case '\f':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = 'f';
                    buffer_idx += 2;
                    break;
                }

                case '\"':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = '\"';
                    buffer_idx += 2;
                    break;
                }

                case '\\':
                {
                    string_buffer[buffer_idx] = '\\';
                    string_buffer[buffer_idx+1] = '\\';
                    buffer_idx += 2;
                    break;
                }

                default:
                {
                    if (code <= 0x1F || (escape_utf8 && (code >= 0x7F)))
                    {
                        if (code <= 0xFFFF)
                        {
                            // escape control characters (0x00..0x1F)
                            detail::snprintf_t<char_type>::one_unicode(string_buffer.data() + buffer_idx,
                                                                static_cast<uint16_t>(code));
                            buffer_idx += 6;
                        }
                        else
                        {
                            detail::snprintf_t<char_type>::two_unicodes(string_buffer.data() + buffer_idx,
                                                                static_cast<uint16_t>(0xD7C0u + (code >> 10u)),
                                                                static_cast<uint16_t>(0xDC00u + (code & 0x3FFu)));
                            buffer_idx += 12;
                        }
                    }
                    else
                    {
                        // copy byte to buffer
                        string_buffer[buffer_idx] = char_traits::to_char_type(static_cast<char_int_type>(code));
                        buffer_idx++;
                    }
                    break;
                }
                }

                if (string_buffer.size() - buffer_idx < 13)
                {
                    output(string_buffer.data(), buffer_idx);
                    buffer_idx = 0;
                }
            }

            if (buffer_idx > 0)
            {
                output(string_buffer.data(), buffer_idx);
                buffer_idx = 0;
            }
        }

        void output(std::initializer_list<char_type> text)
        {
            out->write(text.begin(), text.size());
        }

        void output(const char_type ch)
        {
            out->write(ch);
        }

        void output(const char_type *str)
        {
            const auto size = char_traits::length(str);
            out->write(str, static_cast<std::size_t>(size));
        }

        void output(const char_type *str, unsigned int size)
        {
            out->write(str, static_cast<std::size_t>(size));
        }

    private:
        output_adapter<char_type> *out;
        char_type indent_char;
        string_type indent_string;
        std::array<char_type, 32> number_buffer = {};
        std::array<char_type, 512> string_buffer = {};
        ptrdiff_t buffer_idx = 0;
    };
} // namespace jsonxx
