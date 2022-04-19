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
#include "config_encoding.hpp"
#include "config_stream.hpp"
#include "config_value.hpp"

#include <istream> // std::basic_istream

namespace configor {

namespace detail {

template <typename ValueT>
class basic_parser {
public:
    using boolean_type = typename ValueT::boolean_type;
    using integer_type = typename ValueT::integer_type;
    using float_type   = typename ValueT::float_type;
    using char_type    = typename ValueT::char_type;
    using string_type  = typename ValueT::string_type;
    using size_type    = typename ValueT::size_type;
    using decoder      = encoding::decoder<char_type>;
    using encoder      = encoding::encoder<char_type>;

    virtual void source(std::basic_istream<char_type>& is, decoder src_decoder, encoder target_encoder) = 0;

    virtual token_type scan() = 0;

    virtual void get_boolean(integer_type& out) = 0;
    virtual void get_integer(integer_type& out) = 0;
    virtual void get_float(float_type& out)     = 0;
    virtual void get_string(string_type& out)   = 0;

    virtual error_handler* get_error_handler() = 0;
};

} // namespace detail

} // namespace configor
