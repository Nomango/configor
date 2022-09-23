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

#include <initializer_list>  // std::initializer_list
#include <ios>               // std::noskipws
#include <istream>           // std::basic_istream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits

namespace configor
{

namespace detail
{

template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class basic_parser
{
public:
    using value_type     = _ConfTy;
    using integer_type    = typename _ConfTy::integer_type;
    using float_type      = typename _ConfTy::float_type;
    using char_type       = typename _ConfTy::char_type;
    using string_type     = typename _ConfTy::string_type;
    using istream_type    = std::basic_istream<char_type>;
    using source_encoding = _SourceEncoding<char_type>;
    using target_encoding = _TargetEncoding<char_type>;

    virtual void source(std::basic_istream<char_type>& is, encoding::decoder<char_type> src_decoder,
                        encoding::encoder<char_type> target_encoder) = 0;

    virtual token_type scan() = 0;

    virtual void get_integer(integer_type& out) = 0;
    virtual void get_float(float_type& out)     = 0;
    virtual void get_string(string_type& out)   = 0;

    virtual error_handler* get_error_handler() = 0;

    void parse(value_type& c, istream_type& is)
    {
        try
        {
            source(is, source_encoding::decode, target_encoding::encode);

            do_parse(c, token_type::uninitialized);
            if (scan() != token_type::end_of_input)
                fail(token_type::end_of_input);
        }
        catch (...)
        {
            auto eh = get_error_handler();
            if (eh)
                eh->handle(std::current_exception());
            else
                throw;
        }
    }

protected:
    virtual void do_parse(value_type& c, token_type last_token, bool read_next = true)
    {
        using string_type = typename _ConfTy::string_type;

        token_type token = last_token;
        if (read_next)
        {
            token = scan();
        }

        switch (token)
        {
        case token_type::literal_true:
            c = true;
            break;

        case token_type::literal_false:
            c = false;
            break;

        case token_type::literal_null:
            c = config_value_type::null;
            break;

        case token_type::value_string:
            c = config_value_type::string;
            get_string(*c.raw_value().data.string);
            break;

        case token_type::value_integer:
            c = config_value_type::number_integer;
            get_integer(c.raw_value().data.number_integer);
            break;

        case token_type::value_float:
            c = config_value_type::number_float;
            get_float(c.raw_value().data.number_float);
            break;

        case token_type::begin_array:
            c = config_value_type::array;
            while (true)
            {
                token = scan();

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

                c.raw_value().data.vector->push_back(_ConfTy());
                do_parse(c.raw_value().data.vector->back(), token, false);

                // read ','
                token = scan();
                if (token != token_type::value_separator)
                    break;
            }
            if (token != token_type::end_array)
                fail(token, token_type::end_array);
            break;

        case token_type::begin_object:
            c = config_value_type::object;
            while (true)
            {
                token = scan();
                if (token != token_type::value_string)
                    break;

                string_type key{};
                get_string(key);

                token = scan();
                if (token != token_type::name_separator)
                    break;

                _ConfTy object;
                do_parse(object, token);
                c.raw_value().data.object->emplace(key, object);

                // read ','
                token = scan();
                if (token != token_type::value_separator)
                    break;
            }
            if (token != token_type::end_object)
                fail(token, token_type::end_object);
            break;

        default:
            fail(token);
            break;
        }
    }

    void fail(token_type actual_token, const std::string& msg = "unexpected token")
    {
        detail::fast_ostringstream ss;
        ss << msg << " '" << to_string(actual_token) << "'";
        throw configor_deserialization_error(ss.str());
    }

    void fail(token_type actual_token, token_type expected_token, const std::string& msg = "unexpected token")
    {
        fail(actual_token, msg + ", expect '" + to_string(expected_token) + "', but got");
    }
};

//
// parsable
//

template <typename _Args>
class parsable
{
public:
    using value_type = typename _Args::value_type;
    using char_type   = typename value_type::char_type;
    using string_type = typename value_type::string_type;

    template <typename _CharTy>
    using default_encoding = typename _Args::template default_encoding<_CharTy>;

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding>
    using parser = typename _Args::template parser_type<value_type, _SourceEncoding, _TargetEncoding>;

    // parse from stream
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<parser<_SourceEncoding, _TargetEncoding>, _ParserArgs...>::value>::type>
    static void parse(value_type& c, std::basic_istream<char_type>& is, _ParserArgs&&... args)
    {
        parser<_SourceEncoding, _TargetEncoding> p{ std::forward<_ParserArgs>(args)... };
        p.parse(c, is);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<parser<_SourceEncoding, _TargetEncoding>, _ParserArgs...>::value>::type>
    static value_type parse(std::basic_istream<char_type>& is, _ParserArgs&&... args)
    {
        value_type c;
        parse<_SourceEncoding, _TargetEncoding>(c, is, std::forward<_ParserArgs>(args)...);
        return c;
    }

    // parse from string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<parser<_SourceEncoding, _TargetEncoding>, _ParserArgs...>::value>::type>
    static value_type parse(const string_type& str, _ParserArgs&&... args)
    {
        detail::fast_string_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // parse from c-style string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<parser<_SourceEncoding, _TargetEncoding>, _ParserArgs...>::value>::type>
    static value_type parse(const char_type* str, _ParserArgs&&... args)
    {
        detail::fast_buffer_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // parse from c-style file
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<parser<_SourceEncoding, _TargetEncoding>, _ParserArgs...>::value>::type>
    static value_type parse(std::FILE* file, _ParserArgs&&... args)
    {
        detail::fast_cfile_istreambuf<char_type> buf{ file };
        std::basic_istream<char_type>            is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }
};

}  // namespace detail

}  // namespace configor
