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

#include <ios>          // std::streamsize
#include <ostream>      // std::basic_ostream
#include <type_traits>  // std::char_traits

namespace configor
{

namespace detail
{

template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class basic_serializer
{
public:
    using config_type     = _ConfTy;
    using integer_type    = typename _ConfTy::integer_type;
    using float_type      = typename _ConfTy::float_type;
    using char_type       = typename _ConfTy::char_type;
    using string_type     = typename _ConfTy::string_type;
    using ostream_type    = std::basic_ostream<char_type>;
    using source_encoding = _SourceEncoding<char_type>;
    using target_encoding = _TargetEncoding<char_type>;

    basic_serializer() = default;

    virtual void target(std::basic_ostream<char_type>& os, encoding::decoder<char_type> src_decoder,
                        encoding::encoder<char_type> target_encoder) = 0;

    virtual void next(token_type t) = 0;

    virtual void put_integer(integer_type i)           = 0;
    virtual void put_float(float_type f)               = 0;
    virtual void put_string(const string_type& scalbn) = 0;

    virtual error_handler* get_error_handler() = 0;

    void dump(const config_type& c, ostream_type& os)
    {
        try
        {
            target(os, source_encoding::decode, target_encoding::encode);

            do_dump(c);
            next(token_type::end_of_input);
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
    virtual void do_dump(const config_type& c)
    {
        switch (c.type())
        {
        case config_value_type::object:
        {
            const auto& object = *c.raw_value().data.object;

            next(token_type::begin_object);
            if (object.empty())
            {
                next(token_type::end_object);
                return;
            }

            auto iter = object.cbegin();
            auto size = object.size();
            for (std::size_t i = 0; i < size; ++i, ++iter)
            {
                next(token_type::value_string);
                put_string(iter->first);
                next(token_type::name_separator);

                do_dump(iter->second);

                // not last element
                if (i != size - 1)
                {
                    next(token_type::value_separator);
                }
            }
            next(token_type::end_object);
            return;
        }

        case config_value_type::array:
        {
            next(token_type::begin_array);

            auto& v = *c.raw_value().data.vector;
            if (v.empty())
            {
                next(token_type::end_array);
                return;
            }

            const auto size = v.size();
            for (std::size_t i = 0; i < size; ++i)
            {
                do_dump(v.at(i));
                // not last element
                if (i != size - 1)
                {
                    next(token_type::value_separator);
                }
            }
            next(token_type::end_array);
            return;
        }

        case config_value_type::string:
        {
            next(token_type::value_string);
            put_string(*c.raw_value().data.string);
            return;
        }

        case config_value_type::boolean:
        {
            if (c.raw_value().data.boolean)
            {
                next(token_type::literal_true);
            }
            else
            {
                next(token_type::literal_false);
            }
            return;
        }

        case config_value_type::number_integer:
        {
            next(token_type::value_integer);
            put_integer(c.raw_value().data.number_integer);
            return;
        }

        case config_value_type::number_float:
        {
            next(token_type::value_float);
            put_float(c.raw_value().data.number_float);
            return;
        }

        case config_value_type::null:
        {
            next(token_type::literal_null);
            return;
        }
        }
    }
};

//
// serializable
//

template <typename _Args>
class serializable
{
public:
    using config_type = typename _Args::config_type;
    using char_type   = typename config_type::char_type;
    using string_type = typename config_type::string_type;

    template <typename _CharTy>
    using default_encoding = typename _Args::template default_encoding<_CharTy>;

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding>
    using serializer = typename _Args::template serializer_type<config_type, _SourceEncoding, _TargetEncoding>;

    // dump to stream
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<serializer<_SourceEncoding, _TargetEncoding>, _DumpArgs...>::value>::type>
    static void dump(std::basic_ostream<char_type>& os, const config_type& v, _DumpArgs&&... args)
    {
        serializer<_SourceEncoding, _TargetEncoding> s{ std::forward<_DumpArgs>(args)... };
        s.dump(v, os);
    }

    // dump to string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<serializer<_SourceEncoding, _TargetEncoding>, _DumpArgs...>::value>::type>
    static void dump(string_type& str, const config_type& v, _DumpArgs&&... args)
    {
        detail::fast_string_ostreambuf<char_type> buf{ str };
        std::basic_ostream<char_type>             os{ &buf };
        return dump<_SourceEncoding, _TargetEncoding>(os, v, std::forward<_DumpArgs>(args)...);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename                                  = typename std::enable_if<
                  std::is_constructible<serializer<_SourceEncoding, _TargetEncoding>, _DumpArgs...>::value>::type>
    static string_type dump(const config_type& v, _DumpArgs&&... args)
    {
        string_type result;
        dump<_SourceEncoding, _TargetEncoding>(result, v, std::forward<_DumpArgs>(args)...);
        return result;
    }
};

//
// indent
//

template <typename _CharTy>
class indent
{
public:
    using char_type   = _CharTy;
    using string_type = std::basic_string<char_type>;

    indent(int step, char_type ch)
        : depth_(0)
        , step_(step > 0 ? step : 0)
        , indent_char_(ch)
        , indent_string_()
    {
        reverse(static_cast<size_t>(step_ * 2));
    }

    void operator++()
    {
        ++depth_;
        reverse(static_cast<size_t>(depth_ * step_));
    }

    inline void operator--()
    {
        --depth_;
    }

    void put(std::basic_ostream<char_type>& os) const
    {
        os.write(indent_string_.c_str(), static_cast<std::streamsize>(depth_ * step_));
    }

    void put(std::basic_ostream<char_type>& os, int length)
    {
        reverse(static_cast<size_t>(length));
        os.write(indent_string_.c_str(), static_cast<std::streamsize>(length));
    }

    friend inline std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const indent& i)
    {
        if (i.indent_char_ && i.step_ > 0 && i.depth_ > 0)
        {
            i.put(os);
        }
        return os;
    }

private:
    void reverse(size_t length)
    {
        if (indent_char_)
        {
            if (indent_string_.size() < length)
            {
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

}  // namespace detail

}  // namespace configor
