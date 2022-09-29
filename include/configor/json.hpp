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

#include <algorithm>  // std::for_each
#include <iomanip>    // std::setprecision
#include <ios>        // std::noskipws, std::noshowbase, std::right

namespace configor
{

namespace detail
{
template <typename _ValTy, typename _SourceCharTy>
class json_parser;

template <typename _ValTy, typename _TargetCharTy>
class json_serializer;
}  // namespace detail

template <typename _Args, template <typename> class _DefaultEncoding = encoding::auto_utf>
class basic_json final
    : public detail::serializable<_Args, detail::json_serializer, _DefaultEncoding>
    , public detail::parsable<_Args, detail::json_parser, _DefaultEncoding>
    , public detail::value_maker<basic_value<_Args>>
    , public detail::iostream_wrapper_maker<basic_json<_Args, _DefaultEncoding>, basic_value<_Args>>
{
public:
    using value = basic_value<_Args>;

    using serializer =
        typename detail::serializable<_Args, detail::json_serializer,
                                      _DefaultEncoding>::template serializer_type<typename value::char_type>;

    using parser = typename detail::parsable<_Args, detail::json_parser,
                                             _DefaultEncoding>::template parser_type<typename value::char_type>;
};

using json  = basic_json<value_tplargs>;
using wjson = basic_json<wvalue_tplargs>;

// type traits

template <typename _Ty>
struct is_json : std::false_type
{
};

template <typename _Args, template <typename> class _DefaultEncoding>
struct is_json<basic_json<_Args, _DefaultEncoding>> : std::true_type
{
};

namespace detail
{

template <typename _IntTy>
struct json_hex
{
    const _IntTy i;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os, const json_hex& i)
    {
        os << std::setfill(_CharTy('0')) << std::hex << std::uppercase;
        os << '\\' << 'u' << std::setw(sizeof(i.i)) << i.i;
        os << std::dec << std::nouppercase;
        return os;
    }
};

namespace
{

template <typename _IntTy>
inline json_hex<_IntTy> make_json_hex(const _IntTy i)
{
    return json_hex<_IntTy>{ i };
}

}  // namespace

//
// json_parser
//

template <typename _ValTy, typename _SourceCharTy>
class json_parser : public basic_parser<_ValTy, _SourceCharTy>
{
public:
    using value_type       = _ValTy;
    using source_char_type = _SourceCharTy;
    using target_char_type = typename value_type::char_type;

    using option = std::function<void(json_parser&)>;

    static option with_error_handler(error_handler* eh)
    {
        return [=](json_parser& p) { p.set_error_handler(eh); };
    }

    template <template <class> class _Encoding>
    static option with_encoding()
    {
        return [=](json_parser& p)
        {
            p.template set_source_encoding<_Encoding>();
            p.template set_target_encoding<_Encoding>();
        };
    }

    template <template <class> class _Encoding>
    static option with_source_encoding()
    {
        return [=](json_parser& p) { p.template set_source_encoding<_Encoding>(); };
    }

    template <template <class> class _Encoding>
    static option with_target_encoding()
    {
        return [=](json_parser& p) { p.template set_target_encoding<_Encoding>(); };
    }

    explicit json_parser(std::basic_istream<source_char_type>& is)
        : basic_parser<value_type, source_char_type>(is)
        , is_negative_(false)
        , current_(0)
        , number_integer_(0)
        , number_float_(0)
    {
    }

    inline void prepare(std::initializer_list<option> options)
    {
        std::for_each(options.begin(), options.end(), [&](const option& option) { option(*this); });
    }

    virtual void parse(value_type& c) override
    {
        // read first char
        read_next();
        basic_parser<value_type, source_char_type>::parse(c);
    }

    virtual void get_integer(typename value_type::integer_type& out) override
    {
        out = is_negative_ ? -number_integer_ : number_integer_;
    }

    virtual void get_float(typename value_type::float_type& out) override
    {
        out = is_negative_ ? -number_float_ : number_float_;
    }

    virtual void get_string(typename value_type::string_type& out) override
    {
        scan_string(out);
    }

    virtual token_type scan() override
    {
        skip_spaces();

        if (this->is_.eof())
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

        case '\0':
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

    uint32_t read_next()
    {
        if (this->source_decoder_(this->is_, current_))
        {
            if (!this->is_.good())
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
        if (current_ == '/')
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

                if (this->is_.eof())
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

                if (this->is_.eof())
                {
                    fail("unexpected eof while reading comment");
                }
            }
            skip_spaces();
        }
        else
        {
            fail("unexpected character '/'");
        }
    }

    token_type scan_literal(std::initializer_list<source_char_type> text, token_type result)
    {
        for (const auto ch : text)
        {
            if (ch != std::char_traits<source_char_type>::to_char_type(current_))
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

    void scan_string(typename value_type::string_type& out)
    {
        CONFIGOR_ASSERT(current_ == '\"');

        detail::fast_string_ostreambuf<target_char_type> buf{ out };
        std::basic_ostream<target_char_type>             oss{ &buf };
        while (true)
        {
            read_next();
            if (this->is_.eof())
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

                    this->target_encoder_(oss, codepoint);
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
                this->target_encoder_(oss, current_);
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
        using integer_type = typename value_type::integer_type;

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
        using float_type = typename value_type::float_type;

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
        using float_type = typename value_type::float_type;

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

    inline bool is_digit(source_char_type ch) const
    {
        return source_char_type('0') <= ch && ch <= source_char_type('9');
    }

    inline void fail(const std::string& msg)
    {
        throw configor_deserialization_error(msg);
    }

    template <typename _IntTy>
    inline void fail(const std::string& msg, _IntTy code)
    {
        detail::fast_ostringstream ss;
        ss << msg << " '" << make_json_hex(code) << "'";
        fail(ss.str());
    }

private:
    bool                              is_negative_;
    uint32_t                          current_;
    typename value_type::integer_type number_integer_;
    typename value_type::float_type   number_float_;
};

//
// json_serializer
//

template <typename _ValTy, typename _TargetCharTy>
class json_serializer : public basic_serializer<_ValTy, _TargetCharTy>
{
public:
    using value_type       = _ValTy;
    using source_char_type = typename value_type::char_type;
    using target_char_type = _TargetCharTy;

    using option = std::function<void(json_serializer&)>;

    static option with_indent(uint8_t indent_step, target_char_type indent_char = ' ')
    {
        return [=](json_serializer& s)
        {
            s.indent_       = indent<target_char_type>{ indent_step, indent_char };
            s.pretty_print_ = indent_step > 0;
        };
    }

    static option with_unicode_escaping(bool enabled)
    {
        return [=](json_serializer& s) { s.unicode_escaping_ = enabled; };
    }

    static option with_precision(int precision, std::ios_base::fmtflags floatflags = std::ios_base::fixed)
    {
        return [=](json_serializer& s)
        {
            s.os_.precision(static_cast<std::streamsize>(precision));
            s.os_.setf(floatflags, std::ios_base::floatfield);
        };
    }

    static option with_error_handler(error_handler* eh)
    {
        return [=](json_serializer& s) { s.set_error_handler(eh); };
    }

    template <template <class> class _Encoding>
    static option with_encoding()
    {
        return [=](json_serializer& s)
        {
            s.template set_source_encoding<_Encoding>();
            s.template set_target_encoding<_Encoding>();
        };
    }

    template <template <class> class _Encoding>
    static option with_source_encoding()
    {
        return [=](json_serializer& s) { s.template set_source_encoding<_Encoding>(); };
    }

    template <template <class> class _Encoding>
    static option with_target_encoding()
    {
        return [=](json_serializer& s) { s.template set_target_encoding<_Encoding>(); };
    }

    explicit json_serializer(std::basic_ostream<target_char_type>& os)
        : basic_serializer<value_type, target_char_type>(os)
        , pretty_print_(os.width() > 0)
        , object_or_array_began_(false)
        , unicode_escaping_(false)
        , last_token_(token_type::uninitialized)
        , indent_(static_cast<uint8_t>(os.width()), os.fill())
    {
        this->os_ << std::setprecision(os.precision());
        os.width(0);  // clear width
    }

    inline void prepare(std::initializer_list<option> options)
    {
        std::for_each(options.begin(), options.end(), [&](const option& option) { option(*this); });
        if ((this->os_.flags() & std::ios_base::floatfield) == (std::ios_base::fixed | std::ios_base::scientific))
        {
            // hexfloat is disabled
            this->os_.unsetf(std::ios_base::floatfield);
        }
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
                output_indent();
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
            ++indent_;
            break;
        }
        case token_type::end_array:
        {
            --indent_;
            output_newline();
            output_indent();
            output(']');
            break;
        }
        case token_type::begin_object:
        {
            output('{');
            object_or_array_began_ = true;
            ++indent_;
            break;
        }
        case token_type::end_object:
        {
            --indent_;
            output_newline();
            output_indent();
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

    virtual void put_integer(typename value_type::integer_type i) override
    {
        this->os_ << i;
    }

    virtual void put_float(typename value_type::float_type f) override
    {
        if (std::ceil(f) == std::floor(f))
        {
            // integer
            this->os_ << static_cast<int64_t>(f) << ".0";
        }
        else
        {
            this->os_ << f;
        }
    }

    virtual void put_string(const typename value_type::string_type& s) override
    {
        output('\"');

        fast_string_istreambuf<source_char_type> buf{ s };
        std::basic_istream<source_char_type>     iss{ &buf };

        uint32_t codepoint = 0;
        while (this->source_decoder_(iss, codepoint))
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
                const bool need_escape = (codepoint <= 0x1F || (unicode_escaping_ && codepoint >= 0x7F));
                if (!need_escape)
                {
                    // ASCII or BMP (U+0000...U+007F)
                    this->target_encoder_(this->os_, codepoint);
                }
                else
                {
                    if (codepoint <= 0xFFFF)
                    {
                        // BMP: U+007F...U+FFFF
                        this->os_ << make_json_hex(static_cast<uint16_t>(codepoint));
                    }
                    else
                    {
                        // supplementary planes: U+10000...U+10FFFF
                        uint32_t lead_surrogate = 0, trail_surrogate = 0;
                        encoding::unicode::encode_surrogates(codepoint, lead_surrogate, trail_surrogate);
                        this->os_ << make_json_hex(lead_surrogate) << make_json_hex(trail_surrogate);
                    }
                }

                if (!this->os_.good())
                {
                    fail("encoding failed with codepoint", codepoint);
                }
                break;
            }
            }
        }
        output('\"');
    }

    void output(target_char_type ch)
    {
        this->os_.put(ch);
    }

    void output(std::initializer_list<target_char_type> list)
    {
        this->os_.write(list.begin(), static_cast<std::streamsize>(list.size()));
    }

    void output_indent()
    {
        if (pretty_print_)
            this->os_ << indent_;
    }

    void output_indent(int length)
    {
        if (pretty_print_)
            indent_.put(this->os_, length);
    }

    void output_newline()
    {
        if (pretty_print_)
            this->os_.put('\n');
    }

    inline void fail(const std::string& msg, uint32_t codepoint)
    {
        fast_ostringstream ss;
        ss << msg << " '" << make_json_hex(codepoint) << "'";
        throw configor_serialization_error(ss.str());
    }

private:
    bool                     pretty_print_;
    bool                     object_or_array_began_;
    bool                     unicode_escaping_;
    token_type               last_token_;
    indent<target_char_type> indent_;
};

}  // namespace detail

}  // namespace configor
