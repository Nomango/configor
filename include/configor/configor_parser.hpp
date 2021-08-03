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

//
// configor_lexer
//

template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class basic_lexer
{
public:
    using config_type  = _ConfTy;
    using integer_type = typename _ConfTy::integer_type;
    using float_type   = typename _ConfTy::float_type;
    using char_type    = typename _ConfTy::char_type;
    using string_type  = typename _ConfTy::string_type;

    virtual void source(std::basic_istream<char_type>& is) = 0;

    virtual token_type scan() = 0;

    virtual void get_integer(integer_type& out) = 0;
    virtual void get_float(float_type& out)     = 0;
    virtual void get_string(string_type& out)   = 0;
};

namespace detail
{
template <typename _LexerTy, typename _ConfTy = typename _LexerTy::config_type>
void do_parse_config(_ConfTy& config, _LexerTy& lexer, token_type last_token, bool read_next = true);

inline void parse_fail(token_type actual_token, const std::string& msg = "unexpected token")
{
    detail::fast_ostringstream ss;
    ss << msg << " '" << to_string(actual_token) << "'";
    throw configor_deserialization_error(ss.str());
}

inline void parse_fail(token_type actual_token, token_type expected_token, const std::string& msg = "unexpected token")
{
    parse_fail(actual_token, msg + ", expect '" + to_string(expected_token) + "', but got");
}
}  // namespace detail

template <typename _LexerTy, typename _ConfTy = typename _LexerTy::config_type,
          typename = typename std::enable_if<is_config<_ConfTy>::value>::type>
void parse_config(_ConfTy& c, std::basic_istream<typename _ConfTy::char_type>& is, _LexerTy& lexer, error_handler* eh)
{
    using char_type = typename _ConfTy::char_type;

    lexer.source(is);
    try
    {
        detail::do_parse_config(c, lexer, token_type::uninitialized);
        if (lexer.scan() != token_type::end_of_input)
            detail::parse_fail(token_type::end_of_input);
    }
    catch (...)
    {
        if (eh)
            eh->handle(std::current_exception());
        else
            throw;
    }
}

template <typename _LexerTy, typename _ConfTy = typename _LexerTy::config_type,
          typename = typename std::enable_if<is_config<_ConfTy>::value
                                             && std::is_default_constructible<_LexerTy>::value>::type>
void parse_config(const _ConfTy& c, std::basic_istream<typename _ConfTy::char_type>& is, error_handler* eh = nullptr)
{
    _LexerTy l{};
    parse_config(c, is, l, eh);
}

namespace detail
{

template <typename _LexerTy, typename _ConfTy>
void do_parse_config(_ConfTy& config, _LexerTy& lexer, token_type last_token, bool read_next)
{
    using string_type = typename _ConfTy::string_type;

    token_type token = last_token;
    if (read_next)
    {
        token = lexer.scan();
    }

    switch (token)
    {
    case token_type::literal_true:
        config = true;
        break;

    case token_type::literal_false:
        config = false;
        break;

    case token_type::literal_null:
        config = config_value_type::null;
        break;

    case token_type::value_string:
        config = config_value_type::string;
        lexer.get_string(*config.raw_value().data.string);
        break;

    case token_type::value_integer:
        config = config_value_type::number_integer;
        lexer.get_integer(config.raw_value().data.number_integer);
        break;

    case token_type::value_float:
        config = config_value_type::number_float;
        lexer.get_float(config.raw_value().data.number_float);
        break;

    case token_type::begin_array:
        config = config_value_type::array;
        while (true)
        {
            token = lexer.scan();

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

            config.raw_value().data.vector->push_back(_ConfTy());
            do_parse_config(config.raw_value().data.vector->back(), lexer, token, false);

            // read ','
            token = lexer.scan();
            if (token != token_type::value_separator)
                break;
        }
        if (token != token_type::end_array)
            detail::parse_fail(token, token_type::end_array);
        break;

    case token_type::begin_object:
        config = config_value_type::object;
        while (true)
        {
            token = lexer.scan();
            if (token != token_type::value_string)
                break;

            string_type key{};
            lexer.get_string(key);

            token = lexer.scan();
            if (token != token_type::name_separator)
                break;

            _ConfTy object;
            do_parse_config(object, lexer, token);
            config.raw_value().data.object->emplace(key, object);

            // read ','
            token = lexer.scan();
            if (token != token_type::value_separator)
                break;
        }
        if (token != token_type::end_object)
            detail::parse_fail(token, token_type::end_object);
        break;

    default:
        detail::parse_fail(token);
        break;
    }
}

}  // namespace detail
}  // namespace configor
