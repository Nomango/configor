// Copyright (c) 2018-2020 configor - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// is the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included is
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
#include "configor_conversion.hpp"
#include "configor_declare.hpp"
#include "configor_iterator.hpp"
#include "configor_parser.hpp"
#include "configor_serializer.hpp"
#include "configor_value.hpp"

#include <algorithm>    // std::for_each, std::all_of
#include <type_traits>  // std::enable_if, std::is_same, std::is_integral, std::is_floating_point
#include <utility>      // std::forward, std::declval

namespace configor
{

template <typename _Args>
class basic_config
{
    friend struct detail::iterator<basic_config>;
    friend struct detail::iterator<const basic_config>;

public:
    template <typename T>
    using binder_type = typename _Args::template binder_type<T>;

    template <typename CharT>
    using default_encoding = typename _Args::template default_encoding<CharT>;

    using reader = typename _Args::template reader_type<basic_config>;
    using writer = typename _Args::template writer_type<basic_config>;

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding>
    using parser = typename _Args::template parser_type<basic_config, _SourceEncoding, _TargetEncoding>;

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding>
    using serializer = typename _Args::template serializer_type<basic_config, _SourceEncoding, _TargetEncoding>;

    basic_config(const std::initializer_list<basic_config>& init_list,
                 config_value_type                          exact_type = config_value_type::null)
    {
        bool is_an_object = std::all_of(init_list.begin(), init_list.end(),
                                        [](const basic_config& config)
                                        { return (config.is_array() && config.size() == 2 && config[0].is_string()); });

        if (exact_type != config_value_type::object && exact_type != config_value_type::array)
        {
            exact_type = is_an_object ? config_value_type::object : config_value_type::array;
        }

        if (exact_type == config_value_type::object)
        {
            if (!is_an_object)
                throw configor_type_error("initializer_list is not object type");

            value_ = config_value_type::object;
            std::for_each(init_list.begin(), init_list.end(),
                          [this](const basic_config& config)
                          {
                              value_.data.object->emplace(*((*config.value_.data.vector)[0].value_.data.string),
                                                          (*config.value_.data.vector)[1]);
                          });
        }
        else
        {
            value_ = config_value_type::array;
            value_.data.vector->reserve(init_list.size());
            value_.data.vector->assign(init_list.begin(), init_list.end());
        }
    }

    static inline basic_config object(const std::initializer_list<basic_config>& init_list)
    {
        return basic_config(init_list, config_value_type::object);
    }

    static inline basic_config array(const std::initializer_list<basic_config>& init_list)
    {
        return basic_config(init_list, config_value_type::array);
    }

public:
    // dump to stream
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename = typename std::enable_if<detail::can_serialize<basic_config, _DumpArgs...>::value>::type>
    void dump(std::basic_ostream<char_type>& os, _DumpArgs&&... args) const
    {
        serializer<_SourceEncoding, _TargetEncoding>::dump(*this, os, std::forward<_DumpArgs>(args)...);
    }

    // dump to string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename = typename std::enable_if<detail::can_serialize<basic_config, _DumpArgs...>::value>::type>
    void dump(string_type& str, _DumpArgs&&... args) const
    {
        detail::fast_string_ostreambuf<char_type> buf{ str };
        std::basic_ostream<char_type>             os{ &buf };
        return dump<_SourceEncoding, _TargetEncoding>(os, std::forward<_DumpArgs>(args)...);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename = typename std::enable_if<detail::can_serialize<basic_config, _DumpArgs...>::value>::type>
    string_type dump(_DumpArgs&&... args) const
    {
        string_type result;
        dump<_SourceEncoding, _TargetEncoding>(result, std::forward<_DumpArgs>(args)...);
        return result;
    }

    // parse from stream
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static void parse(basic_config& c, std::basic_istream<char_type>& is, _ParserArgs&&... args)
    {
        parser<_SourceEncoding, _TargetEncoding>::parse(c, is, std::forward<_ParserArgs>(args)...);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(std::basic_istream<char_type>& is, _ParserArgs&&... args)
    {
        basic_config c;
        parse<_SourceEncoding, _TargetEncoding>(c, is, std::forward<_ParserArgs>(args)...);
        return c;
    }

    // parse from string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(const string_type& str, _ParserArgs&&... args)
    {
        detail::fast_string_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // parse from c-style string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(const char_type* str, _ParserArgs&&... args)
    {
        detail::fast_buffer_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // parse from c-style file
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(std::FILE* file, _ParserArgs&&... args)
    {
        detail::fast_cfile_istreambuf<char_type> buf{ file };
        std::basic_istream<char_type>            is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // wrap

    template <typename T, typename = typename std::enable_if<!is_config<T>::value
                                                               && detail::is_configor_getable<basic_config, T>::value
                                                               && !std::is_pointer<T>::value>::type>
    static inline detail::write_configor_wrapper<basic_config, T> wrap(T& v)
    {
        return detail::write_configor_wrapper<basic_config, T>(v);
    }

    template <typename T,
              typename = typename std::enable_if<!std::is_same<basic_config, T>::value
                                                 && detail::has_to_config<basic_config, T>::value>::type>
    static inline detail::read_configor_wrapper<basic_config, T> wrap(const T& v)
    {
        return detail::read_configor_wrapper<basic_config, T>(v);
    }
};

}  // namespace configor
