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
#include "json_encoding.hpp"
#include "json_stream.hpp"
#include "json_value.hpp"

#include <initializer_list>  // std::initializer_list
#include <istream>           // std::basic_istream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits

namespace jsonxx
{

template <typename _BasicJsonTy>
struct parser_args
{
    bool allow_comments = false;  // allow comments
    bool check_document = false;  // only allow object or array type
};

//
// json_lexer
//

enum class token_type
{
    uninitialized,

    literal_true,
    literal_false,
    literal_null,

    value_string,
    value_integer,
    value_float,

    begin_array,
    end_array,

    begin_object,
    end_object,

    name_separator,
    value_separator,

    end_of_input
};

template <typename _BasicJsonTy>
struct json_lexer
{
    using char_type     = typename _BasicJsonTy::char_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;
    using string_type   = typename _BasicJsonTy::string_type;
    using integer_type  = typename _BasicJsonTy::integer_type;
    using float_type    = typename _BasicJsonTy::float_type;
    using encoding_type = typename _BasicJsonTy::encoding_type;
    using args          = parser_args<_BasicJsonTy>;

    json_lexer(std::basic_istream<char_type>& is, const args& args)
        : is_negative_(false)
        , number_integer_(0)
        , number_float_(0)
        , string_buffer_()
        , args_(args)
        , current_(0)
        , fmt_(is)
        , is_(is)
    {
        // read first char
        read_next();
    }

    char_int_type read_next()
    {
        current_ = is_.get();
        return current_;
    }

    void skip_spaces()
    {
        while (current_ == ' ' || current_ == '\t' || current_ == '\n' || current_ == '\r')
            read_next();

        // skip comments
        if (args_.allow_comments && current_ == '/')
        {
            read_next();
            if (current_ == '/')
            {
                // one line comment
                while (true)
                {
                    read_next();
                    if (current_ == '\n' || current_ == '\r')
                    {
                        // end of comment
                        skip_spaces();
                        break;
                    }

                    if (current_ == 0 || current_ == char_traits::eof())
                    {
                        // end of input
                        break;
                    }
                }
            }
            else if (current_ == '*')
            {
                // multiple line comment
                while (true)
                {
                    read_next();
                    if (current_ == '*')
                    {
                        read_next();
                        if (current_ == '/')
                        {
                            // end of comment
                            read_next();
                            break;
                        }
                    }
                }
                skip_spaces();
            }
            else
            {
                fail("unexpected character '/'");
            }
        }
    }

    token_type scan()
    {
        skip_spaces();

        token_type result = token_type::uninitialized;
        switch (current_)
        {
        case '[':
            result = token_type::begin_array;
            break;
        case ']':
            result = token_type::end_array;
            break;
        case '{':
            result = token_type::begin_object;
            break;
        case '}':
            result = token_type::end_object;
            break;
        case ':':
            result = token_type::name_separator;
            break;
        case ',':
            result = token_type::value_separator;
            break;

        case 't':
            return scan_literal({ 't', 'r', 'u', 'e' }, token_type::literal_true);
        case 'f':
            return scan_literal({ 'f', 'a', 'l', 's', 'e' }, token_type::literal_false);
        case 'n':
            return scan_literal({ 'n', 'u', 'l', 'l' }, token_type::literal_null);

        case '\"':
            return scan_string();

        case '-':
        case '+':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return scan_number();

        // case '\0':
        case char_traits::eof():
            return token_type::end_of_input;

        default:
        {
            fail("unexpected character", current_);
        }
        }

        // skip next char
        read_next();

        return result;
    }

    token_type scan_literal(std::initializer_list<char_type> text, token_type result)
    {
        for (const auto ch : text)
        {
            if (ch != char_traits::to_char_type(current_))
            {
                detail::fast_ostringstream ss;
                ss << "unexpected character '" << static_cast<char>(current_) << "'";
                ss << " (expected literal '";
                for (const auto ch : text)
                    ss.put(static_cast<char>(ch));
                ss << "')";
                fail(ss.str());
            }
            read_next();
        }
        return result;
    }

    token_type scan_string()
    {
        JSONXX_ASSERT(current_ == '\"');

        string_buffer_.clear();

        detail::fast_string_ostreambuf<char_type> buf{ string_buffer_ };
        std::basic_ostream<char_type>             oss{ &buf };
        while (true)
        {
            const auto ch = read_next();
            switch (ch)
            {
            case char_traits::eof():
            {
                fail("unexpected end of string");
            }

            case '\"':
            {
                // skip last `\"`
                read_next();
                return token_type::value_string;
            }

            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
            case 0x0E:
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
            case 0x1F:
            {
                fail("invalid control character", current_);
            }

            case '\\':
            {
                switch (read_next())
                {
                case '\"':
                    oss << '\"';
                    break;
                case '\\':
                    oss << '\\';
                    break;
                case '/':
                    oss << '/';
                    break;
                case 'b':
                    oss << '\b';
                    break;
                case 'f':
                    oss << '\f';
                    break;
                case 'n':
                    oss << '\n';
                    break;
                case 'r':
                    oss << '\r';
                    break;
                case 't':
                    oss << '\t';
                    break;

                case 'u':
                {
                    uint32_t codepoint = read_escaped_codepoint();
                    if (encoding::unicode::is_lead_surrogate(codepoint))
                    {
                        if (read_next() != '\\' || read_next() != 'u')
                        {
                            fail("lead surrogate must be followed by trail surrogate, but got", current_);
                        }

                        const auto lead_surrogate  = codepoint;
                        const auto trail_surrogate = read_escaped_codepoint();

                        if (!encoding::unicode::is_trail_surrogate(trail_surrogate))
                        {
                            fail("surrogate U+D800...U+DBFF must be followed by U+DC00...U+DFFF, but got",
                                 trail_surrogate);
                        }
                        codepoint = encoding::unicode::decode_surrogates(lead_surrogate, trail_surrogate);
                    }

                    encoding_type::encode(oss, codepoint);
                    if (!oss.good())
                    {
                        fail("encoding failed with codepoint", codepoint);
                    }
                    break;
                }

                default:
                {
                    fail("invalid escaped character", current_);
                }
                }
                break;
            }

            default:
            {
                oss << char_traits::to_char_type(ch);
            }
            }
        }
    }

    token_type scan_number()
    {
        is_negative_    = false;
        number_integer_ = 0;

        if (current_ == '-' || current_ == '+')
        {
            is_negative_ = (current_ == '-');
            read_next();
        }

        if (current_ == '0')
        {
            read_next();
            if (current_ == '.')
                return scan_float();

            if (current_ == 'e' || current_ == 'E')
                fail("invalid exponent");

            // zero
            return token_type::value_integer;
        }
        return scan_integer();
    }

    token_type scan_integer()
    {
        JSONXX_ASSERT(is_digit(current_));

        number_integer_ = static_cast<integer_type>(current_ - '0');
        while (true)
        {
            const auto ch = read_next();
            if (ch == '.' || ch == 'e' || ch == 'E')
                return scan_float();

            if (is_digit(ch))
                number_integer_ = number_integer_ * 10 + static_cast<integer_type>(ch - '0');
            else
                break;
        }
        return token_type::value_integer;
    }

    token_type scan_float()
    {
        number_float_ = static_cast<float_type>(number_integer_);

        if (current_ == 'e' || current_ == 'E')
            return scan_exponent();

        JSONXX_ASSERT(current_ == '.');

        if (!is_digit(read_next()))
            fail("invalid float number, got", current_);

        float_type fraction = static_cast<float_type>(0.1);
        number_float_ += static_cast<float_type>(static_cast<uint32_t>(current_ - '0')) * fraction;

        while (true)
        {
            const auto ch = read_next();
            if (ch == 'e' || ch == 'E')
                return scan_exponent();

            if (is_digit(ch))
            {
                fraction *= static_cast<float_type>(0.1);
                number_float_ += static_cast<float_type>(static_cast<uint32_t>(current_ - '0')) * fraction;
            }
            else
                break;
        }
        return token_type::value_float;
    }

    token_type scan_exponent()
    {
        JSONXX_ASSERT(current_ == 'e' || current_ == 'E');

        read_next();

        const bool invalid = (is_digit(current_) && current_ != '0') || (current_ == '-') || (current_ == '+');
        if (!invalid)
            fail("invalid exponent number, got", current_);

        float_type base = 10;
        if (current_ == '+')
        {
            read_next();
        }
        else if (current_ == '-')
        {
            base = static_cast<float_type>(0.1);
            read_next();
        }

        uint32_t exponent = static_cast<uint32_t>(current_ - '0');
        while (is_digit(read_next()))
        {
            exponent = (exponent * 10) + static_cast<uint32_t>(current_ - '0');
        }

        float_type power = 1;
        for (; exponent; exponent >>= 1, base *= base)
            if (exponent & 1)
                power *= base;

        number_float_ *= power;
        return token_type::value_float;
    }

    integer_type token_to_integer() const
    {
        return is_negative_ ? -number_integer_ : number_integer_;
    }

    float_type token_to_float() const
    {
        return is_negative_ ? -number_float_ : number_float_;
    }

    const string_type& token_to_string() const
    {
        return string_buffer_;
    }

    uint32_t read_escaped_codepoint()
    {
        uint32_t code = 0;
        for (const auto factor : { 12, 8, 4, 0 })
        {
            const auto ch = read_next();
            if (ch >= '0' && ch <= '9')
            {
                code += ((ch - '0') << factor);
            }
            else if (ch >= 'A' && ch <= 'F')
            {
                code += ((ch - 'A' + 10) << factor);
            }
            else if (ch >= 'a' && ch <= 'f')
            {
                code += ((ch - 'a' + 10) << factor);
            }
            else
            {
                fail("'\\u' must be followed by 4 hex digits, but got", ch);
            }
        }
        return code;
    }

    inline bool is_digit(char_type ch) const
    {
        return char_type('0') <= ch && ch <= char_type('9');
    }

    inline void fail(const std::string& msg)
    {
        throw json_deserialization_error(msg);
    }

    template <typename _IntTy>
    inline void fail(const std::string& msg, _IntTy code)
    {
        detail::fast_ostringstream ss;
        ss << msg << " '" << detail::serialize_hex(code) << "'";
        fail(ss.str());
    }

private:
    bool         is_negative_;
    integer_type number_integer_;
    float_type   number_float_;
    string_type  string_buffer_;

    const args& args_;

    char_int_type                    current_;
    detail::format_keeper<char_type> fmt_;
    std::basic_istream<char_type>&   is_;
};

//
// json_parser
//

template <typename _BasicJsonTy>
struct json_parser
{
    using char_type  = typename _BasicJsonTy::char_type;
    using lexer_type = json_lexer<_BasicJsonTy>;
    using args       = parser_args<_BasicJsonTy>;

    json_parser(std::basic_istream<char_type>& is, const args& args)
        : lexer_(is, args)
        , last_token_(token_type::uninitialized)
    {
    }

    void parse(_BasicJsonTy& json)
    {
        parse_value(json);

        if (get_token() != token_type::end_of_input)
            fail(token_type::end_of_input);
    }

private:
    token_type get_token()
    {
        last_token_ = lexer_.scan();
        return last_token_;
    }

    void parse_value(_BasicJsonTy& json, bool read_next = true)
    {
        token_type token = last_token_;
        if (read_next)
        {
            token = get_token();
        }

        switch (token)
        {
        case token_type::literal_true:
            json.value_ = true;
            break;

        case token_type::literal_false:
            json.value_ = false;
            break;

        case token_type::literal_null:
            json = json_type::null;
            break;

        case token_type::value_string:
            json = lexer_.token_to_string();
            break;

        case token_type::value_integer:
            json = lexer_.token_to_integer();
            break;

        case token_type::value_float:
            json = lexer_.token_to_float();
            break;

        case token_type::begin_array:
            json = json_type::array;
            while (true)
            {
                const auto token = get_token();

                bool is_end = false;
                switch (token)
                {
                case token_type::literal_true:
                case token_type::literal_false:
                case token_type::literal_null:
                case token_type::value_string:
                case token_type::value_integer:
                case token_type::value_float:
                case token_type::begin_array:
                case token_type::begin_object:
                    break;
                default:
                    is_end = true;
                    break;
                }
                if (is_end)
                    break;

                json.value_.data.vector->push_back(_BasicJsonTy());
                parse_value(json.value_.data.vector->back(), false);

                // read ','
                if (get_token() != token_type::value_separator)
                    break;
            }
            if (last_token_ != token_type::end_array)
                fail(token_type::end_array);
            break;

        case token_type::begin_object:
            json = json_type::object;
            while (true)
            {
                if (get_token() != token_type::value_string)
                    break;

                const auto key = lexer_.token_to_string();
                if (get_token() != token_type::name_separator)
                    break;

                _BasicJsonTy object;
                parse_value(object);
                json.value_.data.object->insert(std::make_pair(key, object));

                // read ','
                if (get_token() != token_type::value_separator)
                    break;
            }
            if (last_token_ != token_type::end_object)
                fail(token_type::end_object);
            break;

        default:
            fail();
            break;
        }
    }

    inline void fail(token_type expected_token, const std::string& msg = "unexpected token")
    {
        fail(msg + ", expect '" + token_str(expected_token) + "', but got");
    }

    inline void fail(const std::string& msg = "unexpected token")
    {
        detail::fast_ostringstream ss;
        ss << msg << " '" << token_str(last_token_) << "'";
        throw json_deserialization_error(ss.str());
    }

    inline const char* token_str(token_type token) const
    {
        switch (token)
        {
        case token_type::uninitialized:
            return "uninitialized";
        case token_type::literal_true:
            return "literal_true";
        case token_type::literal_false:
            return "literal_false";
        case token_type::literal_null:
            return "literal_null";
        case token_type::value_string:
            return "value_string";
        case token_type::value_integer:
            return "value_integer";
        case token_type::value_float:
            return "value_float";
        case token_type::begin_array:
            return "begin_array";
        case token_type::end_array:
            return "end_array";
        case token_type::begin_object:
            return "begin_object";
        case token_type::end_object:
            return "end_object";
        case token_type::name_separator:
            return "name_separator";
        case token_type::value_separator:
            return "value_separator";
        case token_type::end_of_input:
            return "end_of_input";
        }
        return "unknown";
    }

private:
    lexer_type lexer_;
    token_type last_token_;
};

}  // namespace jsonxx
