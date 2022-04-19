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

#include <ios>     // std::streamsize
#include <ostream> // std::basic_ostream
#include <stack>   // std::stack

namespace configor {

namespace detail {

template <typename ValueT>
class basic_serializer {
public:
    using boolean_type = typename ValueT::boolean_type;
    using integer_type = typename ValueT::integer_type;
    using float_type   = typename ValueT::float_type;
    using char_type    = typename ValueT::char_type;
    using string_type  = typename ValueT::string_type;
    using size_type    = typename ValueT::size_type;

    virtual void next(token_type token, size_type size = 0) = 0;

    virtual void put_boolean(const boolean_type b) = 0;
    virtual void put_integer(const integer_type i) = 0;
    virtual void put_float(const float_type f)     = 0;
    virtual void put_string(const string_type& s)  = 0;
};

template <typename ValueT>
class stream_serializer : public basic_serializer<ValueT> {
public:
    using char_type = typename ValueT::char_type;
    using decoder   = encoding::decoder<char_type>;
    using encoder   = encoding::encoder<char_type>;

    virtual void target(std::basic_ostream<char_type>& os, decoder src_decoder, encoder target_encoder) = 0;
};

template <typename ValueT>
class config_value_serializer : basic_serializer<ValueT> {
public:
    using boolean_type = typename ValueT::boolean_type;
    using integer_type = typename ValueT::integer_type;
    using float_type   = typename ValueT::float_type;
    using char_type    = typename ValueT::char_type;
    using string_type  = typename ValueT::string_type;
    using size_type    = typename ValueT::size_type;
    using value_type   = ValueT;

    virtual void target(value_type& v) {
        value_ = &v;
        stack_.push(value_);
    }

    virtual void next(token_type token, size_type size = 0) {
        switch (token) {
        case token_type::none:
            throw make_serialization_error(token);
        case token_type::value_null:
            if (stack_.empty())
                must_meet(token, token_type::eof);

            return "value_null";
        case token_type::value_boolean:
            expect_ = token;
            return "value_boolean";
        case token_type::value_integer:
            return "value_integer";
        case token_type::value_float:
            return "value_float";
        case token_type::value_string:
            return "value_string";
        case token_type::array_begin:
            return "array_begin";
        case token_type::array_end:
            return "array_end";
        case token_type::object_begin:
            return "object_begin";
        case token_type::object_end:
            return "object_end";
        case token_type::eof:
            return "EOF";
        }
    }

    virtual void put_boolean(const boolean_type b) override { must_meet(token_type::value_boolean); }
    virtual void put_integer(const integer_type i) = 0;
    virtual void put_float(const float_type f)     = 0;
    virtual void put_string(const string_type& s)  = 0;

private:
    void must_meet(token_type token) { must_meet(token, expect_); }

    void must_meet(token_type token, token_type expect) {
        if (token != expect)
            throw make_serialization_error(token, expect);
    }

private:
    value_type*             value_;
    token_type              expect_;
    std::stack<value_type*> stack_;
};

//
// indent
//

template <typename CharT>
class indent {
public:
    using char_type   = CharT;
    using string_type = std::basic_string<char_type>;

    indent(int step, char_type ch) : depth_(0), step_(step > 0 ? step : 0), indent_char_(ch), indent_string_() {
        reverse(static_cast<size_t>(step_ * 2));
    }

    void operator++() {
        ++depth_;
        reverse(static_cast<size_t>(depth_ * step_));
    }

    inline void operator--() { --depth_; }

    void put(std::basic_ostream<char_type>& os) const {
        os.write(indent_string_.c_str(), static_cast<std::streamsize>(depth_ * step_));
    }

    void put(std::basic_ostream<char_type>& os, int length) {
        reverse(static_cast<size_t>(length));
        os.write(indent_string_.c_str(), static_cast<std::streamsize>(length));
    }

    friend inline std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const indent& i) {
        if (i.indent_char_ && i.step_ > 0 && i.depth_ > 0) {
            i.put(os);
        }
        return os;
    }

private:
    void reverse(size_t length) {
        if (indent_char_) {
            if (indent_string_.size() < length) {
                indent_string_.resize(length + static_cast<size_t>(step_ * 2), indent_char_);
            }
        }
    }

private:
    int         depth_;
    int         step_;
    char_type   indent_char_;
    string_type indent_string_;
};

} // namespace detail

} // namespace configor
