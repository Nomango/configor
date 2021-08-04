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
#include "configor.hpp"

#include <iomanip>  // std::setprecision, std::right, std::noshowbase

namespace configor
{

namespace detail
{
template <typename _JsonTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class json_lexer;

template <typename _JsonTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class json_serializer;

template <typename _JsonTy>
struct json_lexer_args;

template <typename _JsonTy>
struct json_serializer_args;
}  // namespace detail

struct json_args : config_args
{
    template <class _JsonTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using lexer_type = detail::json_lexer<_JsonTy, _SourceEncoding, _TargetEncoding>;

    template <class _ConfTy>
    using lexer_args_type = detail::json_lexer_args<_ConfTy>;

    template <class _JsonTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using serializer_type = detail::json_serializer<_JsonTy, _SourceEncoding, _TargetEncoding>;

    template <class _ConfTy>
    using serializer_args_type = detail::json_serializer_args<_ConfTy>;

    template <typename _CharTy>
    using default_encoding = encoding::auto_utf<_CharTy>;
};

struct wjson_args : json_args
{
    using char_type = wchar_t;
};

using json  = basic_config<json_args>;
using wjson = basic_config<wjson_args>;

// type traits

template <typename _JsonTy>
struct is_json : std::false_type
{
};

template <typename _Args>
struct is_json<basic_config<_Args>>
{
    using type = basic_config<_Args>;

    template <typename>
    struct dummy;

    static const bool value =
        std::is_same<typename type::template lexer_type<dummy, dummy>, detail::json_lexer<type, dummy, dummy>>::value
        && std::is_same<typename type::template serializer_type<dummy, dummy>,
                        detail::json_serializer<type, dummy, dummy>>::value;
};

#define JSON_BIND(value_type, ...) CONFIGOR_BIND_WITH_CONF(json, value_type, __VA_ARGS__)
#define WJSON_BIND(value_type, ...) CONFIGOR_BIND_WITH_CONF(wjson, value_type, __VA_ARGS__)

// json serialization

template <typename _SerialTy, typename _JsonTy = typename _SerialTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
void dump_config(const _JsonTy& j, std::basic_ostream<typename _JsonTy::char_type>& os,
                 const typename _SerialTy::args& args = {}, error_handler* eh = nullptr)
{
    _SerialTy s{ args };
    dump_config<_SerialTy>(j, os, s, eh);
}

template <typename _SerialTy, typename _JsonTy = typename _SerialTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
typename _JsonTy::string_type dump_config(const _JsonTy& j, const typename _SerialTy::args& args = {},
                                          error_handler* eh = nullptr)
{
    using char_type   = typename _JsonTy::char_type;
    using string_type = typename _JsonTy::string_type;

    string_type                               result;
    detail::fast_string_ostreambuf<char_type> buf{ result };
    std::basic_ostream<char_type>             os{ &buf };
    dump_config<_SerialTy>(j, os, args, eh);
    return result;
}

template <typename _SerialTy, typename _JsonTy = typename _SerialTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
typename _JsonTy::string_type dump_config(const _JsonTy& j, unsigned int indent,
                                          typename _JsonTy::char_type indent_char = ' ', bool escape_unicode = false,
                                          error_handler* eh = nullptr)
{
    using char_type   = typename _JsonTy::char_type;
    using string_type = typename _JsonTy::string_type;

    typename _SerialTy::args args;
    args.indent         = indent;
    args.indent_char    = indent_char;
    args.escape_unicode = escape_unicode;
    return dump_config<_SerialTy>(j, args, eh);
}

template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
std::basic_ostream<typename _JsonTy::char_type>& operator<<(std::basic_ostream<typename _JsonTy::char_type>& os,
                                                            const _JsonTy&                                   j)
{
    using serializer_type = typename _JsonTy::template serializer_type<encoding::auto_utf, encoding::auto_utf>;
    typename serializer_type::args args;
    args.indent      = static_cast<unsigned int>(os.width());
    args.indent_char = os.fill();
    args.precision   = static_cast<int>(os.precision());

    os.width(0);

    dump_config<serializer_type>(j, os, args);
    return os;
}

// parse functions

template <typename _LexerTy, typename _JsonTy = typename _LexerTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
void parse_config(_JsonTy& j, std::basic_istream<typename _JsonTy::char_type>& is,
                  const typename _LexerTy::args& args = {}, error_handler* eh = nullptr)
{
    _LexerTy lexer{ args };
    parse_config<_LexerTy>(j, is, lexer, eh);

    if (args.check_document && !j.is_array() && !j.is_object())
    {
        // must be object or array
        try
        {
            std::string name = j.type_name();
            throw configor_deserialization_error("invalid document type '" + name + "'");
        }
        catch (...)
        {
            if (eh)
                eh->handle(std::current_exception());
            else
                throw;
        }
    }
}

template <typename _LexerTy, typename _JsonTy = typename _LexerTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
void parse_config(_JsonTy& j, const typename _JsonTy::string_type& str, const typename _LexerTy::args& args = {},
                  error_handler* eh = nullptr)
{
    using char_type = typename _JsonTy::char_type;

    detail::fast_string_istreambuf<char_type> buf{ str };
    std::basic_istream<char_type>             is{ &buf };
    parse_config<_LexerTy>(j, is, args, eh);
}

template <typename _LexerTy, typename _JsonTy = typename _LexerTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
void parse_config(_JsonTy& j, const typename _JsonTy::char_type* str, const typename _LexerTy::args& args = {},
                  error_handler* eh = nullptr)
{
    using char_type = typename _JsonTy::char_type;

    detail::fast_buffer_istreambuf<char_type> buf{ str };
    std::basic_istream<char_type>             is{ &buf };
    parse_config<_LexerTy>(j, is, args, eh);
}

template <typename _LexerTy, typename _JsonTy = typename _LexerTy::config_type,
          typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
void parse_config(_JsonTy& j, std::FILE* file, const typename _LexerTy::args& args = {}, error_handler* eh = nullptr)
{
    using char_type = typename _JsonTy::char_type;

    detail::fast_cfile_istreambuf<char_type> buf{ file };
    std::basic_istream<char_type>            is{ &buf };
    parse_config<_LexerTy>(j, is, args, eh);
}

template <typename _JsonTy, typename = typename std::enable_if<is_json<_JsonTy>::value>::type>
std::basic_istream<typename _JsonTy::char_type>& operator>>(std::basic_istream<typename _JsonTy::char_type>& is,
                                                            _JsonTy&                                         j)
{
    using lexer_type = typename _JsonTy::template lexer_type<encoding::auto_utf, encoding::auto_utf>;
    parse_config<lexer_type>(j, is);
    return is;
}

namespace detail
{

//
// json lexer
//

template <typename _JsonTy>
struct json_lexer_args
{
    bool allow_comments = false;  // allow comments
    bool check_document = false;  // only allow object or array type
};

template <typename _JsonTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class json_lexer : public basic_lexer<_JsonTy, _SourceEncoding, _TargetEncoding>
{
public:
    using char_type       = typename _JsonTy::char_type;
    using char_traits     = std::char_traits<char_type>;
    using char_int_type   = typename char_traits::int_type;
    using string_type     = typename _JsonTy::string_type;
    using integer_type    = typename _JsonTy::integer_type;
    using float_type      = typename _JsonTy::float_type;
    using source_encoding = _SourceEncoding<char_type>;
    using target_encoding = _TargetEncoding<char_type>;

    using args = json_lexer_args<_JsonTy>;

    explicit json_lexer(const args& args)
        : is_(nullptr)
        , is_negative_(false)
        , number_integer_(0)
        , number_float_(0)
        , args_(args)
        , current_(0)
    {
        is_ >> std::noskipws;
    }

    virtual void source(std::basic_istream<char_type>& is) override
    {
        is_.rdbuf(is.rdbuf());
        // read first char
        read_next();
    }

    virtual void get_integer(integer_type& out) override
    {
        out = is_negative_ ? -number_integer_ : number_integer_;
    }

    virtual void get_float(float_type& out) override
    {
        out = is_negative_ ? -number_float_ : number_float_;
    }

    virtual void get_string(string_type& out) override
    {
        scan_string(out);
    }

    virtual token_type scan() override
    {
        skip_spaces();

        if (is_.eof())
            return token_type::end_of_input;

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
            // lazy load
            return token_type::value_string;

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

        default:
        {
            fail("unexpected character", current_);
        }
        }

        // skip next char
        read_next();
        return result;
    }

    char_int_type read_next()
    {
        if (source_encoding::decode(is_, current_))
        {
            if (!is_.good())
            {
                fail("decoding failed with codepoint", current_);
            }
        }
        else
        {
            current_ = 0;
        }
        return current_;
    }

    void skip_spaces()
    {
        while (current_ == ' ' || current_ == '\t' || current_ == '\n' || current_ == '\r')
            read_next();

        // skip comments
        if (args_.allow_comments && current_ == '/')
        {
            skip_comments();
        }
    }

    void skip_comments()
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

                if (is_.eof())
                {
                    break;
                }
            }
        }
        else if (current_ == '*')
        {
            // multiple line comment
            while (true)
            {
                if (read_next() == '*')
                {
                    if (read_next() == '/')
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

    void scan_string(string_type& out)
    {
        CONFIGOR_ASSERT(current_ == '\"');

        detail::fast_string_ostreambuf<char_type> buf{ out };
        std::basic_ostream<char_type>             oss{ &buf };
        while (true)
        {
            read_next();
            if (is_.eof())
            {
                fail("unexpected end of string");
            }

            switch (current_)
            {
            case '\"':
            {
                // end of string
                // skip last `\"`
                read_next();
                return;
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

                    target_encoding::encode(oss, codepoint);
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
                target_encoding::encode(oss, current_);
                if (!oss.good())
                {
                    fail("encoding failed with codepoint", current_);
                }
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
        CONFIGOR_ASSERT(is_digit(current_));

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

        CONFIGOR_ASSERT(current_ == '.');

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
        CONFIGOR_ASSERT(current_ == 'e' || current_ == 'E');

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
        throw configor_deserialization_error(msg);
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

    const args& args_;

    uint32_t                      current_;
    std::basic_istream<char_type> is_;
};

//
// json serializer
//

template <typename _JsonTy>
struct json_serializer_args
{
    using char_type  = typename _JsonTy::char_type;
    using float_type = typename _JsonTy::float_type;

    unsigned int indent         = 0;
    char_type    indent_char    = ' ';
    bool         escape_unicode = false;
    int          precision      = std::numeric_limits<float_type>::digits10 + 1;
};

template <typename _JsonTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class json_serializer : public basic_serializer<_JsonTy, _SourceEncoding, _TargetEncoding>
{
public:
    using char_type       = typename _JsonTy::char_type;
    using char_traits     = std::char_traits<char_type>;
    using char_int_type   = typename char_traits::int_type;
    using string_type     = typename _JsonTy::string_type;
    using integer_type    = typename _JsonTy::integer_type;
    using float_type      = typename _JsonTy::float_type;
    using source_encoding = _SourceEncoding<char_type>;
    using target_encoding = _TargetEncoding<char_type>;

    using args = json_serializer_args<_JsonTy>;

    explicit json_serializer(const args& args)
        : os_(nullptr)
        , object_or_array_began_(false)
        , pretty_print_(args.indent > 0)
        , depth_(0)
        , last_token_(token_type::uninitialized)
        , args_(args)
        , indent_()
    {
        os_ << std::setprecision(args_.precision);
        os_ << std::right;
        os_ << std::noshowbase;

        if (pretty_print_)
        {
            indent_.init(args_.indent_char);
        }
    }

    virtual void target(std::basic_ostream<char_type>& os) override
    {
        os_.rdbuf(os.rdbuf());
    }

    virtual void next(token_type token) override
    {
        if (object_or_array_began_)
        {
            object_or_array_began_ = false;
            switch (token)
            {
            case token_type::end_array:
            case token_type::end_object:
                // empty object or array
                break;
            case token_type::literal_true:
            case token_type::literal_false:
            case token_type::literal_null:
            case token_type::value_string:
            case token_type::value_integer:
            case token_type::value_float:
            case token_type::begin_array:
            case token_type::begin_object:
                output_newline();
                break;
            default:
                // unexpected
                break;
            }
        }

        if (pretty_print_ && last_token_ != token_type::name_separator)
        {
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
                output_indent(depth_ * args_.indent);
                break;
            default:
                break;
            }
        }

        switch (token)
        {
        case token_type::uninitialized:
            break;
        case token_type::literal_true:
        {
            output({ 't', 'r', 'u', 'e' });
            break;
        }
        case token_type::literal_false:
        {
            output({ 'f', 'a', 'l', 's', 'e' });
            break;
        }
        case token_type::literal_null:
        {
            output({ 'n', 'u', 'l', 'l' });
            break;
        }
        case token_type::value_string:
            break;
        case token_type::value_integer:
            break;
        case token_type::value_float:
            break;
        case token_type::begin_array:
        {
            output('[');
            object_or_array_began_ = true;
            depth_++;
            break;
        }
        case token_type::end_array:
        {
            --depth_;
            output_newline();
            output_indent(depth_ * args_.indent);
            output(']');
            break;
        }
        case token_type::begin_object:
        {
            output('{');
            object_or_array_began_ = true;
            depth_++;
            break;
        }
        case token_type::end_object:
        {
            --depth_;
            output_newline();
            output_indent(depth_ * args_.indent);
            output('}');
            break;
        }
        case token_type::name_separator:
        {
            output(':');
            output_indent(1);
            break;
        }
        case token_type::value_separator:
        {
            output(',');
            output_newline();
            break;
        }
        case token_type::end_of_input:
            break;
        }

        last_token_ = token;
    }

    virtual void put_integer(integer_type i) override
    {
        os_ << serialize_integer(i);
    }

    virtual void put_float(float_type f) override
    {
        os_ << serialize_float(f);
    }

    virtual void put_string(const string_type& s) override
    {
        output('\"');

        fast_string_istreambuf<char_type> buf{ s };
        std::basic_istream<char_type>     iss{ &buf };

        uint32_t codepoint = 0;
        while (source_encoding::decode(iss, codepoint))
        {
            if (!iss.good())
            {
                fail("unexpected character", codepoint);
            }

            switch (codepoint)
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
                const bool need_escape = ((codepoint <= 0x1F || (args_.escape_unicode && codepoint >= 0x7F))
                                          && encoding::is_unicode_encoding<target_encoding>::value);
                if (!need_escape)
                {
                    // ASCII or BMP (U+0000...U+007F)
                    target_encoding::encode(os_, codepoint);
                }
                else
                {
                    if (codepoint <= 0xFFFF)
                    {
                        // BMP: U+007F...U+FFFF
                        os_ << serialize_hex(static_cast<uint16_t>(codepoint));
                    }
                    else
                    {
                        // supplementary planes: U+10000...U+10FFFF
                        uint32_t lead_surrogate = 0, trail_surrogate = 0;
                        encoding::unicode::encode_surrogates(codepoint, lead_surrogate, trail_surrogate);
                        os_ << serialize_hex(lead_surrogate) << serialize_hex(trail_surrogate);
                    }
                }

                if (!os_.good())
                {
                    fail("encoding failed with codepoint", codepoint);
                }
                break;
            }
            }
        }
        output('\"');
    }

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
        if (pretty_print_ && size)
            os_ << indent_ * size;
    }

    void output_newline()
    {
        if (pretty_print_)
            os_.put('\n');
    }

    inline void fail(const std::string& msg, uint32_t codepoint)
    {
        fast_ostringstream ss;
        ss << msg << " '" << serialize_hex(codepoint) << "'";
        throw configor_serialization_error(ss.str());
    }

private:
    bool         object_or_array_began_;
    const bool   pretty_print_;
    unsigned int depth_;
    const args&  args_;

    token_type last_token_;

    indent<string_type>           indent_;
    std::basic_ostream<char_type> os_;
};

}  // namespace detail

}  // namespace configor
