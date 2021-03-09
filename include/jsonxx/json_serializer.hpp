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
#include <cstdio>  // snprintf_s
#include <cassert>  // assert
#include <type_traits>  // std::char_traits
#include <ios>  // std::basic_ostream
#include <array>  // std::array
#include <algorithm>  // std::none_of

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
        virtual void write(const _CharTy *str)
        {
            const auto size = char_traits::length(str);
            write(str, static_cast<std::size_t>(size));
        }
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

        json_serializer(output_adapter<char_type> *out, const char_type indent_char)
            : out(out), indent_char(indent_char), indent_string(32, indent_char)
        {
        }

        void dump(
            const _BasicJsonTy &json,
            const bool pretty_print,
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
                    out->write("{}");
                    return;
                }

                if (pretty_print)
                {
                    out->write("{\n");

                    const auto new_indent = current_indent + indent_step;
                    if (indent_string.size() < new_indent)
                    {
                        indent_string.resize(indent_string.size() * 2, indent_char);
                    }

                    auto iter = object.cbegin();
                    const auto size = object.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        out->write(indent_string.c_str(), new_indent);
                        out->write('\"');
                        out->write(iter->first.c_str());
                        out->write("\": ");
                        dump(iter->second, true, indent_step, new_indent);

                        // not last element
                        if (i != size - 1)
                            out->write(",\n");
                    }

                    out->write('\n');
                    out->write(indent_string.c_str(), current_indent);
                    out->write('}');
                }
                else
                {
                    out->write('{');

                    auto iter = object.cbegin();
                    const auto size = object.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        out->write('\"');
                        out->write(iter->first.c_str());
                        out->write("\":");
                        dump(iter->second, false, indent_step, current_indent);

                        // not last element
                        if (i != size - 1)
                            out->write(',');
                    }

                    out->write('}');
                }

                return;
            }

            case json_type::array:
            {
                auto &vector = *json.value_.data.vector;

                if (vector.empty())
                {
                    out->write("[]");
                    return;
                }

                if (pretty_print)
                {
                    out->write("[\n");

                    const auto new_indent = current_indent + indent_step;
                    if (indent_string.size() < new_indent)
                    {
                        indent_string.resize(indent_string.size() * 2, indent_char);
                    }

                    auto iter = vector.cbegin();
                    const auto size = vector.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        out->write(indent_string.c_str(), new_indent);
                        dump(*iter, true, indent_step, new_indent);

                        // not last element
                        if (i != size - 1)
                            out->write(",\n");
                    }

                    out->write('\n');
                    out->write(indent_string.c_str(), current_indent);
                    out->write(']');
                }
                else
                {
                    out->write('[');

                    auto iter = vector.cbegin();
                    const auto size = vector.size();
                    for (std::size_t i = 0; i < size; ++i, ++iter)
                    {
                        dump(*iter, false, indent_step, current_indent);
                        // not last element
                        if (i != size - 1)
                            out->write(',');
                    }

                    out->write(']');
                }

                return;
            }

            case json_type::string:
            {
                out->write('\"');
                dump_string(*json.value_.data.string);
                out->write('\"');
                return;
            }

            case json_type::boolean:
            {
                if (json.value_.data.boolean)
                {
                    out->write("true");
                }
                else
                {
                    out->write("false");
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
                out->write("null");
                return;
            }
            }
        }

        void dump_integer(integer_type val)
        {
            if (val == 0)
            {
                out->write('0');
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

            out->write(&(*next));
        }

        void dump_float(float_type val)
        {
            const auto digits = std::numeric_limits<float_type>::max_digits10;
            const auto len = ::std::snprintf(number_buffer.data(), number_buffer.size(), "%.*g", digits, val);

            // check len
            assert(len > 0);
            assert(number_buffer.size() > static_cast<std::size_t>(len));

            out->write(number_buffer.data(), static_cast<std::size_t>(len));

            // determine if need to append ".0"
            if (std::none_of(number_buffer.begin(), number_buffer.begin() + len + 1, [](char c) { return c == '.'; }))
            {
                out->write(".0");
            }
        }

        void dump_string(const string_type &val)
        {
            for (const auto &ch : val)
            {
                switch (ch)
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
                    const auto code = static_cast<uint32_t>(ch);
                    if (code <= 0x1F)
                    {
                        // escape control characters (0x00..0x1F)
                        std::snprintf(string_buffer.data() + buffer_idx, 7, "\\u%04X", uint16_t(code));
                        buffer_idx += 6;
                    }
                    else
                    {
                        string_buffer[buffer_idx] = ch;
                        buffer_idx++;
                    }
                    break;
                }
                }

                if (string_buffer.size() - buffer_idx < 7)
                {
                    out->write(string_buffer.data(), buffer_idx);
                    buffer_idx = 0;
                }
            }

            if (buffer_idx > 0)
            {
                out->write(string_buffer.data(), buffer_idx);
                buffer_idx = 0;
            }
        }

    private:
        output_adapter<char_type> *out;
        char_type indent_char;
        string_type indent_string;
        std::array<char_type, 21> number_buffer = {};
        std::array<char, 512> string_buffer = {};
        ptrdiff_t buffer_idx = 0;
    };
} // namespace jsonxx
