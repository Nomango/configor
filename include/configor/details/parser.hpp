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
#include "encoding.hpp"
#include "stream.hpp"
#include "token.hpp"
#include "value.hpp"

#include <initializer_list>  // std::initializer_list
#include <istream>           // std::basic_istream
#include <locale>            // std::locale

namespace configor
{

namespace detail
{

template <typename _ValTy, typename _SourceCharTy>
class basic_parser
{
public:
    using value_type       = _ValTy;
    using source_char_type = _SourceCharTy;
    using target_char_type = typename value_type::char_type;

    basic_parser(std::basic_istream<source_char_type>& is)
        : is_(is.rdbuf())
        , err_handler_(nullptr)
        , source_decoder_(nullptr)
        , target_encoder_(nullptr)
    {
        is_.unsetf(std::ios_base::skipws);
        is_.imbue(std::locale(std::locale::classic(), is.getloc(), std::locale::collate | std::locale::ctype));
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
        source_decoder_ = _Encoding<source_char_type>::decode;
    }

    template <template <class> class _Encoding>
    inline void set_target_encoding()
    {
        target_encoder_ = _Encoding<target_char_type>::encode;
    }

protected:
    virtual token_type scan() = 0;

    virtual void get_integer(typename value_type::integer_type& out) = 0;
    virtual void get_float(typename value_type::float_type& out)     = 0;
    virtual void get_string(typename value_type::string_type& out)   = 0;

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
            c = value_constant::null;
            break;

        case token_type::value_string:
            c = value_constant::string;
            get_string(*c.data().string);
            break;

        case token_type::value_integer:
            c = value_constant::integer;
            get_integer(c.data().integer);
            break;

        case token_type::value_float:
            c = value_constant::floating;
            get_float(c.data().floating);
            break;

        case token_type::begin_array:
            c = value_constant::array;
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

                c.data().vector->push_back(_ValTy());
                do_parse(c.data().vector->back(), token, false);

                // read ','
                token = scan();
                if (token != token_type::value_separator)
                    break;
            }
            if (token != token_type::end_array)
                fail(token, token_type::end_array);
            break;

        case token_type::begin_object:
            c = value_constant::object;
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
                c.data().object->emplace(key, object);

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
    std::basic_istream<source_char_type> is_;
    error_handler*                       err_handler_;
    encoding::decoder<source_char_type>  source_decoder_;
    encoding::encoder<target_char_type>  target_encoder_;
};

//
// parsable
//

template <typename _Args, template <class, class> class _ParserTy, template <class> class _DefaultEncoding>
class parsable
{
public:
    using value_type = basic_value<_Args>;

    template <typename _SourceCharTy>
    using parser_type = _ParserTy<value_type, _SourceCharTy>;

    template <typename _SourceCharTy>
    using parser_option = typename parser_type<_SourceCharTy>::option;

    // parse from stream
    template <typename _SourceCharTy>
    static void parse(value_type& c, std::basic_istream<_SourceCharTy>& is,
                      std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        parser_type<_SourceCharTy> p{ is };
        p.template set_source_encoding<_DefaultEncoding>();
        p.template set_target_encoding<_DefaultEncoding>();
        p.prepare(options);
        p.parse(c);
    }

    template <typename _SourceCharTy>
    static value_type parse(std::basic_istream<_SourceCharTy>&                  is,
                            std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        value_type c;
        parse(c, is, options);
        return c;
    }

    // parse from string
    template <typename _SourceCharTy>
    static void parse(value_type& c, const typename _Args::template string_type<_SourceCharTy>& str,
                      std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        detail::fast_string_istreambuf<_SourceCharTy> buf{ str };
        std::basic_istream<_SourceCharTy>             is{ &buf };
        parse(c, is, options);
    }

    template <typename _SourceCharTy>
    static value_type parse(const typename _Args::template string_type<_SourceCharTy>& str,
                            std::initializer_list<parser_option<_SourceCharTy>>        options = {})
    {
        detail::fast_string_istreambuf<_SourceCharTy> buf{ str };
        std::basic_istream<_SourceCharTy>             is{ &buf };
        return parse(is, options);
    }

    // parse from c-style string
    template <typename _SourceCharTy>
    static void parse(value_type& c, const _SourceCharTy* str,
                      std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        detail::fast_buffer_istreambuf<_SourceCharTy> buf{ str };
        std::basic_istream<_SourceCharTy>             is{ &buf };
        parse(c, is, options);
    }

    template <typename _SourceCharTy>
    static value_type parse(const _SourceCharTy* str, std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        detail::fast_buffer_istreambuf<_SourceCharTy> buf{ str };
        std::basic_istream<_SourceCharTy>             is{ &buf };
        return parse(is, options);
    }

    // parse from c-style file
    template <typename _SourceCharTy = typename value_type::char_type>
    static void parse(value_type& c, std::FILE* file, std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        detail::fast_cfile_istreambuf<_SourceCharTy> buf{ file };
        std::basic_istream<_SourceCharTy>            is{ &buf };
        parse(c, is, options);
    }

    template <typename _SourceCharTy = typename value_type::char_type>
    static value_type parse(std::FILE* file, std::initializer_list<parser_option<_SourceCharTy>> options = {})
    {
        detail::fast_cfile_istreambuf<_SourceCharTy> buf{ file };
        std::basic_istream<_SourceCharTy>            is{ &buf };
        return parse(is, options);
    }
};

}  // namespace detail

}  // namespace configor
