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

#include <functional>        // std::function
#include <initializer_list>  // std::initializer_list
#include <ios>               // std::noskipws
#include <istream>           // std::basic_istream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits

namespace configor
{

namespace detail
{

template <typename _ValTy>
class basic_parser
{
public:
    using value_type   = _ValTy;
    using integer_type = typename _ValTy::integer_type;
    using float_type   = typename _ValTy::float_type;
    using char_type    = typename _ValTy::char_type;
    using string_type  = typename _ValTy::string_type;
    using istream_type = std::basic_istream<char_type>;

    basic_parser(istream_type& is)
        : is_(nullptr)
        , err_handler_(nullptr)
        , source_decoder_(nullptr)
        , target_encoder_(nullptr)
    {
        is_.rdbuf(is.rdbuf());
        is_.copyfmt(is);
    }

    virtual void parse(value_type& c)
    {
        try
        {
            do_parse(c, token_type::uninitialized);
            if (scan() != token_type::end_of_input)
                fail(token_type::end_of_input);
        }
        catch (...)
        {
            if (err_handler_)
                err_handler_->handle(std::current_exception());
            else
                throw;
        }
    }

    inline void set_error_handler(configor::error_handler* eh)
    {
        err_handler_ = eh;
    }

    template <template <class> class _Encoding>
    inline void set_source_encoding()
    {
        source_decoder_ = _Encoding<char_type>::decode;
    }

    template <template <class> class _Encoding>
    inline void set_target_encoding()
    {
        target_encoder_ = _Encoding<char_type>::encode;
    }

protected:
    virtual token_type scan() = 0;

    virtual void get_integer(integer_type& out) = 0;
    virtual void get_float(float_type& out)     = 0;
    virtual void get_string(string_type& out)   = 0;

    virtual void do_parse(value_type& c, token_type last_token, bool read_next = true)
    {
        using string_type = typename _ValTy::string_type;

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

                c.raw_value().data.vector->push_back(_ValTy());
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

                _ValTy object;
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

protected:
    std::basic_istream<char_type> is_;
    error_handler*                err_handler_;
    encoding::decoder<char_type>  source_decoder_;
    encoding::encoder<char_type>  target_encoder_;
};

//
// parsable
//

template <typename _Args>
class parsable
{
public:
    using value_type  = typename _Args::value_type;
    using char_type   = typename value_type::char_type;
    using string_type = typename value_type::string_type;

    template <typename _CharTy>
    using default_encoding = typename _Args::template default_encoding<_CharTy>;

    using parser        = typename _Args::template parser_type<value_type>;
    using parser_option = std::function<void(parser&)>;

    // parse from stream
    static void parse(value_type& c, std::basic_istream<char_type>& is,
                      std::initializer_list<parser_option> options = {})
    {
        parser p{ is };
        p.template set_source_encoding<default_encoding>();
        p.template set_target_encoding<default_encoding>();
        for (const auto& option : options)
        {
            option(p);
        }
        p.parse(c);
    }

    static value_type parse(std::basic_istream<char_type>& is, std::initializer_list<parser_option> options = {})
    {
        value_type c;
        parse(c, is, options);
        return c;
    }

    // parse from string
    static void parse(value_type& c, const string_type& str, std::initializer_list<parser_option> options = {})
    {
        detail::fast_string_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        parse(c, is, options);
    }

    static value_type parse(const string_type& str, std::initializer_list<parser_option> options = {})
    {
        detail::fast_string_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse(is, options);
    }

    // parse from c-style string
    static void parse(value_type& c, const char_type* str, std::initializer_list<parser_option> options = {})
    {
        detail::fast_buffer_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        parse(c, is, options);
    }

    static value_type parse(const char_type* str, std::initializer_list<parser_option> options = {})
    {
        detail::fast_buffer_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse(is, options);
    }

    // parse from c-style file
    static void parse(value_type& c, std::FILE* file, std::initializer_list<parser_option> options = {})
    {
        detail::fast_cfile_istreambuf<char_type> buf{ file };
        std::basic_istream<char_type>            is{ &buf };
        parse(c, is, options);
    }

    static value_type parse(std::FILE* file, std::initializer_list<parser_option> options = {})
    {
        detail::fast_cfile_istreambuf<char_type> buf{ file };
        std::basic_istream<char_type>            is{ &buf };
        return parse(is, options);
    }
};

}  // namespace detail

}  // namespace configor
