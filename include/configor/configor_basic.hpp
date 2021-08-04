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
    template <typename _Ty>
    using allocator_type = typename _Args::template allocator_type<_Ty>;
    using boolean_type   = typename _Args::boolean_type;
    using integer_type   = typename _Args::integer_type;
    using float_type     = typename _Args::float_type;
    using char_type      = typename _Args::char_type;
    using string_type =
        typename _Args::template string_type<char_type, std::char_traits<char_type>, allocator_type<char_type>>;
    using array_type = typename _Args::template array_type<basic_config, allocator_type<basic_config>>;
    using object_type =
        typename _Args::template object_type<string_type, basic_config, std::less<string_type>,
                                             allocator_type<std::pair<const string_type, basic_config>>>;
    using value_type = detail::config_value<basic_config>;

    template <template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using lexer_type = typename _Args::template lexer_type<basic_config<_Args>, _SourceEncoding, _TargetEncoding>;

    template <template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using serializer_type =
        typename _Args::template serializer_type<basic_config<_Args>, _SourceEncoding, _TargetEncoding>;

    using parse_args = typename _Args::template lexer_args_type<basic_config>;
    using dump_args  = typename _Args::template serializer_args_type<basic_config>;

    template <typename _CharTy>
    using default_encoding = typename _Args::template default_encoding<_CharTy>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using iterator               = detail::iterator<basic_config>;
    using const_iterator         = detail::iterator<const basic_config>;
    using reverse_iterator       = detail::reverse_iterator<iterator>;
    using const_reverse_iterator = detail::reverse_iterator<const_iterator>;

public:
    basic_config(std::nullptr_t = nullptr) {}

    basic_config(const config_value_type type)
        : value_(type)
    {
    }

    basic_config(const basic_config& other)
        : value_(other.value_)
    {
    }

    basic_config(basic_config&& other) noexcept
        : value_(std::move(other.value_))
    {
        // invalidate payload
        other.value_.type        = config_value_type::null;
        other.value_.data.object = nullptr;
    }

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

    template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
              typename = typename std::enable_if<!std::is_same<basic_config, _UTy>::value
                                                 && detail::has_to_config<basic_config, _UTy>::value>::type>
    basic_config(_CompatibleTy&& value)
    {
        config_bind<_UTy>::to_config(*this, std::forward<_CompatibleTy>(value));
    }

    static inline basic_config object(const std::initializer_list<basic_config>& init_list)
    {
        return basic_config(init_list, config_value_type::object);
    }

    static inline basic_config array(const std::initializer_list<basic_config>& init_list)
    {
        return basic_config(init_list, config_value_type::array);
    }

    inline bool is_object() const
    {
        return value_.type == config_value_type::object;
    }

    inline bool is_array() const
    {
        return value_.type == config_value_type::array;
    }

    inline bool is_string() const
    {
        return value_.type == config_value_type::string;
    }

    inline bool is_bool() const
    {
        return value_.type == config_value_type::boolean;
    }

    inline bool is_integer() const
    {
        return value_.type == config_value_type::number_integer;
    }

    inline bool is_float() const
    {
        return value_.type == config_value_type::number_float;
    }

    inline bool is_number() const
    {
        return is_integer() || is_float();
    }

    inline bool is_null() const
    {
        return value_.type == config_value_type::null;
    }

    inline config_value_type type() const
    {
        return value_.type;
    }

    inline const char* type_name() const
    {
        return to_string(type());
    }

    inline void swap(basic_config& rhs)
    {
        value_.swap(rhs.value_);
    }

public:
    inline iterator begin()
    {
        iterator iter(this);
        iter.set_begin();
        return iter;
    }
    inline const_iterator begin() const
    {
        return cbegin();
    }
    inline const_iterator cbegin() const
    {
        const_iterator iter(this);
        iter.set_begin();
        return iter;
    }
    inline iterator end()
    {
        iterator iter(this);
        iter.set_end();
        return iter;
    }
    inline const_iterator end() const
    {
        return cend();
    }
    inline const_iterator cend() const
    {
        const_iterator iter(this);
        iter.set_end();
        return iter;
    }
    inline reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }
    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    inline const_reverse_iterator crbegin() const
    {
        return rbegin();
    }
    inline reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }
    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
    inline const_reverse_iterator crend() const
    {
        return rend();
    }

public:
    inline size_type size() const
    {
        switch (type())
        {
        case config_value_type::null:
            return 0;
        case config_value_type::array:
            return value_.data.vector->size();
        case config_value_type::object:
            return value_.data.object->size();
        default:
            return 1;
        }
    }

    inline bool empty() const
    {
        if (is_null())
            return true;

        if (is_object())
            return value_.data.object->empty();

        if (is_array())
            return value_.data.vector->empty();

        return false;
    }

    inline iterator find(const typename object_type::key_type& key)
    {
        if (is_object())
        {
            iterator iter(this);
            iter.object_it_ = value_.data.object->find(key);
            return iter;
        }
        return end();
    }

    inline const_iterator find(const typename object_type::key_type& key) const
    {
        if (is_object())
        {
            const_iterator iter(this);
            iter.object_it_ = value_.data.object->find(key);
            return iter;
        }
        return cend();
    }

    inline size_type count(const typename object_type::key_type& key) const
    {
        return is_object() ? value_.data.object->count(key) : 0;
    }

    inline size_type erase(const typename object_type::key_type& key)
    {
        if (!is_object())
        {
            throw configor_invalid_key("cannot use erase() with non-object value");
        }
        return value_.data.object->erase(key);
    }

    inline void erase(const size_type index)
    {
        if (!is_array())
        {
            throw configor_invalid_key("cannot use erase() with non-array value");
        }
        value_.data.vector->erase(value_.data.vector->begin() + static_cast<difference_type>(index));
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy pos)
    {
        _IterTy result = end();

        switch (type())
        {
        case config_value_type::object:
        {
            result.object_it_ = value_.data.object->erase(pos.object_it_);
            break;
        }

        case config_value_type::array:
        {
            result.array_it_ = value_.data.vector->erase(pos.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    inline _IterTy erase(_IterTy first, _IterTy last)
    {
        _IterTy result = end();

        switch (type())
        {
        case config_value_type::object:
        {
            result.object_it_ = value_.data.object->erase(first.object_it_, last.object_it_);
            break;
        }

        case config_value_type::array:
        {
            result.array_it_ = value_.data.vector->erase(first.array_it_, last.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    inline void push_back(basic_config&& config)
    {
        if (!is_null() && !is_array())
        {
            throw configor_type_error("cannot use push_back() with non-array value");
        }

        if (is_null())
        {
            value_ = config_value_type::array;
        }

        value_.data.vector->push_back(std::move(config));
    }

    inline basic_config& operator+=(basic_config&& config)
    {
        push_back(std::move(config));
        return (*this);
    }

    inline void clear()
    {
        switch (type())
        {
        case config_value_type::number_integer:
        {
            value_.data.number_integer = 0;
            break;
        }

        case config_value_type::number_float:
        {
            value_.data.number_float = static_cast<float_type>(0.0);
            break;
        }

        case config_value_type::boolean:
        {
            value_.data.boolean = false;
            break;
        }

        case config_value_type::string:
        {
            value_.data.string->clear();
            break;
        }

        case config_value_type::array:
        {
            value_.data.vector->clear();
            break;
        }

        case config_value_type::object:
        {
            value_.data.object->clear();
            break;
        }

        default:
            break;
        }
    }

private:
    // GET value functions

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_config, _Ty>::value>::type>
    inline _Ty do_get(detail::priority<2>) const
    {
        return *this;
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_config<basic_config, _Ty>::value>::type>
    inline _Ty do_get(detail::priority<1>) const
    {
        return config_bind<_Ty>::from_config(*this);
    }

    template <typename _Ty,
              typename = typename std::enable_if<std::is_default_constructible<_Ty>::value
                                                 && detail::has_default_from_config<basic_config, _Ty>::value>::type>
    inline _Ty do_get(detail::priority<0>) const
    {
        _Ty value{};
        config_bind<_Ty>::from_config(*this, value);
        return value;
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_config, _Ty>::value>::type>
    inline _Ty& do_get(_Ty& value, detail::priority<2>) const
    {
        return (value = *this);
    }

    template <typename _Ty,
              typename = typename std::enable_if<std::is_default_constructible<_Ty>::value
                                                 && detail::has_default_from_config<basic_config, _Ty>::value>::type>
    inline _Ty& do_get(_Ty& value, detail::priority<1>) const
    {
        config_bind<_Ty>::from_config(*this, value);
        return value;
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_config<basic_config, _Ty>::value>::type>
    inline _Ty& do_get(_Ty& value, detail::priority<0>) const
    {
        value = config_bind<_Ty>::from_config(*this);
        return value;
    }

public:
    template <typename _Ty, typename _UTy = typename detail::remove_cvref<_Ty>::type>
    inline auto get() const
        -> decltype(std::declval<const basic_config&>().template do_get<_UTy>(detail::priority<2>{}))
    {
        return do_get<_UTy>(detail::priority<2>{});
    }

    template <typename _Ty>
    inline auto get(_Ty& value) const
        -> decltype(std::declval<const basic_config&>().template do_get<_Ty>(std::declval<_Ty&>(),
                                                                             detail::priority<2>{}))
    {
        return do_get<_Ty>(value, detail::priority<2>{});
    }

    template <typename _Ty>
    auto try_get(_Ty& value) const -> typename std::enable_if<
        !std::is_void<decltype(std::declval<const basic_config&>().template get<_Ty>(std::declval<_Ty&>()))>::value,
        bool>::type
    {
        try
        {
            get<_Ty>(value);
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    boolean_type as_bool() const
    {
        switch (type())
        {
        case config_value_type::number_integer:
            return value_.data.number_integer != 0;
        case config_value_type::number_float:
            return value_.data.number_float != 0;
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
            return !empty();
        case config_value_type::boolean:
            return value_.data.boolean;
        case config_value_type::null:
            break;
        }
        return false;
    }

    integer_type as_integer() const
    {
        switch (type())
        {
        case config_value_type::number_integer:
            return value_.data.number_integer;
        case config_value_type::number_float:
            return static_cast<integer_type>(value_.data.number_float);
        case config_value_type::boolean:
            return value_.data.boolean ? integer_type(1) : integer_type(0);
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
        case config_value_type::null:
            throw detail::make_conversion_error(type(), config_value_type::number_integer);
        }
        return 0;
    }

    float_type as_float() const
    {
        switch (type())
        {
        case config_value_type::number_integer:
            return static_cast<float_type>(value_.data.number_integer);
        case config_value_type::number_float:
            return value_.data.number_float;
        case config_value_type::boolean:
            return value_.data.boolean ? float_type(1) : float_type(0);
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
        case config_value_type::null:
            throw detail::make_conversion_error(type(), config_value_type::number_integer);
        }
        return 0;
    }

    string_type as_string() const
    {
        switch (type())
        {
        case config_value_type::string:
            return *value_.data.string;
        case config_value_type::number_integer:
        case config_value_type::number_float:
        case config_value_type::boolean:
            return dump(*this);
        case config_value_type::array:
        case config_value_type::object:
            throw detail::make_conversion_error(type(), config_value_type::number_integer);
        case config_value_type::null:
            break;
        }
        return string_type{};
    }

public:
    // operator= functions

    inline basic_config& operator=(basic_config rhs)
    {
        swap(rhs);
        return (*this);
    }

public:
    // operator[] functions

    inline basic_config& operator[](const size_type index)
    {
        if (is_null())
        {
            value_ = config_value_type::array;
        }

        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array object");
        }

        if (index >= value_.data.vector->size())
        {
            value_.data.vector->insert(value_.data.vector->end(), index - value_.data.vector->size() + 1,
                                       basic_config());
        }
        return (*value_.data.vector)[index];
    }

    inline basic_config& operator[](const size_type index) const
    {
        return at(index);
    }

    inline basic_config& operator[](const typename object_type::key_type& key)
    {
        if (is_null())
        {
            value_ = config_value_type::object;
        }

        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object type");
        }
        return (*value_.data.object)[key];
    }

    inline basic_config& operator[](const typename object_type::key_type& key) const
    {
        return at(key);
    }

    inline basic_config& at(const size_type index) const
    {
        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array type");
        }

        if (index >= value_.data.vector->size())
        {
            throw std::out_of_range("operator[] index out of range");
        }
        return (*value_.data.vector)[index];
    }

    template <typename _CharTy>
    inline basic_config& operator[](_CharTy* key)
    {
        if (is_null())
        {
            value_ = config_value_type::object;
        }

        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }
        return (*value_.data.object)[key];
    }

    template <typename _CharTy>
    inline basic_config& operator[](_CharTy* key) const
    {
        return at(key);
    }

    inline basic_config& at(const typename object_type::key_type& key) const
    {
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }

        auto iter = value_.data.object->find(key);
        if (iter == value_.data.object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

    template <typename _CharTy>
    inline basic_config& at(_CharTy* key) const
    {
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }

        auto iter = value_.data.object->find(key);
        if (iter == value_.data.object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

public:
    // conversion

    template <typename _Ty, typename = typename std::enable_if<!is_config<_Ty>::value
                                                               && detail::is_configor_getable<basic_config, _Ty>::value
                                                               && !std::is_pointer<_Ty>::value>::type>
    inline operator _Ty() const
    {
        return get<_Ty>();
    }

    // to_config functions

    template <typename _BoolTy, typename = typename std::enable_if<std::is_same<_BoolTy, boolean_type>::value>::type>
    friend inline void to_config(basic_config& c, _BoolTy value)
    {
        c.value_.type         = config_value_type::boolean;
        c.value_.data.boolean = value;
    }

    friend inline void to_config(basic_config& c, integer_type value)
    {
        c.value_.type                = config_value_type::number_integer;
        c.value_.data.number_integer = value;
    }

    template <typename _IntegerUTy, typename std::enable_if<!std::is_same<_IntegerUTy, boolean_type>::value
                                                                && std::is_integral<_IntegerUTy>::value,
                                                            int>::type = 0>
    friend inline void to_config(basic_config& c, _IntegerUTy value)
    {
        c.value_.type                = config_value_type::number_integer;
        c.value_.data.number_integer = static_cast<integer_type>(value);
    }

    friend inline void to_config(basic_config& c, float_type value)
    {
        c.value_.type              = config_value_type::number_float;
        c.value_.data.number_float = value;
    }

    template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
    friend inline void to_config(basic_config& c, _FloatingTy value)
    {
        c.value_.type              = config_value_type::number_float;
        c.value_.data.number_float = static_cast<float_type>(value);
    }

    friend inline void to_config(basic_config& c, const string_type& value)
    {
        c.value_.type        = config_value_type::string;
        c.value_.data.string = c.value_.template create<string_type>(value);
    }

    friend inline void to_config(basic_config& c, const array_type& value)
    {
        c.value_.type        = config_value_type::array;
        c.value_.data.vector = c.value_.template create<array_type>(value);
    }

    friend inline void to_config(basic_config& c, const object_type& value)
    {
        c.value_.type        = config_value_type::object;
        c.value_.data.object = c.value_.template create<object_type>(value);
    }

    // from_config functions

    template <typename _BoolTy, typename = typename std::enable_if<std::is_same<_BoolTy, boolean_type>::value>::type>
    friend inline void from_config(const basic_config& c, _BoolTy& value)
    {
        if (!c.is_bool())
            throw detail::make_conversion_error(c.type(), config_value_type::boolean);
        value = c.value_.data.boolean;
    }

    friend inline void from_config(const basic_config& c, integer_type& value)
    {
        if (!c.is_integer())
            throw detail::make_conversion_error(c.type(), config_value_type::number_integer);
        value = c.value_.data.number_integer;
    }

    template <typename _IntegerUTy, typename std::enable_if<!std::is_same<_IntegerUTy, boolean_type>::value
                                                                && std::is_integral<_IntegerUTy>::value,
                                                            int>::type = 0>
    friend inline void from_config(const basic_config& c, _IntegerUTy& value)
    {
        if (!c.is_integer())
            throw detail::make_conversion_error(c.type(), config_value_type::number_integer);
        value = static_cast<_IntegerUTy>(c.value_.data.number_integer);
    }

    friend inline void from_config(const basic_config& c, float_type& value)
    {
        if (!c.is_float())
            throw detail::make_conversion_error(c.type(), config_value_type::number_float);
        value = c.value_.data.number_float;
    }

    template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
    friend inline void from_config(const basic_config& c, _FloatingTy& value)
    {
        if (!c.is_float())
            throw detail::make_conversion_error(c.type(), config_value_type::number_float);
        value = static_cast<_FloatingTy>(c.value_.data.number_float);
    }

    friend inline void from_config(const basic_config& c, string_type& value)
    {
        if (!c.is_string())
            throw detail::make_conversion_error(c.type(), config_value_type::string);
        value = *c.value_.data.string;
    }

    friend inline void from_config(const basic_config& c, array_type& value)
    {
        if (!c.is_array())
            throw detail::make_conversion_error(c.type(), config_value_type::array);
        value.assign((*c.value_.data.vector).begin(), (*c.value_.data.vector).end());
    }

    friend inline void from_config(const basic_config& c, object_type& value)
    {
        if (!c.is_object())
            throw detail::make_conversion_error(c.type(), config_value_type::object);
        value = c.value_.data.object;
    }

public:
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs>
    auto dump(_DumpArgs&&... args) const
        -> decltype(dump_config<serializer_type<_SourceEncoding, _TargetEncoding>>(std::declval<const basic_config&>(),
                                                                                   std::forward<_DumpArgs>(args)...))
    {
        return dump_config<serializer_type<_SourceEncoding, _TargetEncoding>>(*this, std::forward<_DumpArgs>(args)...);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParseArgs>
    static auto parse(_ParseArgs&&... args) ->
        typename detail::get_last<decltype(parse_config<lexer_type<_SourceEncoding, _TargetEncoding>>(
                                      std::declval<basic_config&>(), std::forward<_ParseArgs>(args)...)),
                                  basic_config>::type
    {
        basic_config c;
        parse_config<lexer_type<_SourceEncoding, _TargetEncoding>>(c, std::forward<_ParseArgs>(args)...);
        return c;
    }

    template <typename _Ty, typename = typename std::enable_if<!is_config<_Ty>::value
                                                               && detail::is_configor_getable<basic_config, _Ty>::value
                                                               && !std::is_pointer<_Ty>::value>::type>
    static inline detail::write_configor_wrapper<basic_config, _Ty> wrap(_Ty& v)
    {
        return detail::write_configor_wrapper<basic_config, _Ty>(v);
    }

    template <typename _Ty,
              typename = typename std::enable_if<!std::is_same<basic_config, _Ty>::value
                                                 && detail::has_to_config<basic_config, _Ty>::value>::type>
    static inline detail::read_configor_wrapper<basic_config, _Ty> wrap(const _Ty& v)
    {
        return detail::read_configor_wrapper<basic_config, _Ty>(v);
    }

public:
    // eq functions

    friend inline bool operator==(const basic_config& lhs, const basic_config& rhs)
    {
        return lhs.value_ == rhs.value_;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs == basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) == rhs;
    }

    // ne functions

    friend inline bool operator!=(const basic_config& lhs, const basic_config& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs != basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) != rhs;
    }

    // lt functions

    friend bool operator<(const basic_config& lhs, const basic_config& rhs)
    {
        const auto lhs_type = lhs.type();
        const auto rhs_type = rhs.type();

        if (lhs_type == rhs_type)
        {
            switch (lhs_type)
            {
            case config_value_type::array:
                return (*lhs.value_.data.vector) < (*rhs.value_.data.vector);

            case config_value_type::object:
                return (*lhs.value_.data.object) < (*rhs.value_.data.object);

            case config_value_type::null:
                return false;

            case config_value_type::string:
                return (*lhs.value_.data.string) < (*rhs.value_.data.string);

            case config_value_type::boolean:
                return (lhs.value_.data.boolean < rhs.value_.data.boolean);

            case config_value_type::number_integer:
                return (lhs.value_.data.number_integer < rhs.value_.data.number_integer);

            case config_value_type::number_float:
                return (lhs.value_.data.number_float < rhs.value_.data.number_float);

            default:
                return false;
            }
        }
        else if (lhs_type == config_value_type::number_integer && rhs_type == config_value_type::number_float)
        {
            return (static_cast<float_type>(lhs.value_.data.number_integer) < rhs.value_.data.number_float);
        }
        else if (lhs_type == config_value_type::number_float && rhs_type == config_value_type::number_integer)
        {
            return (lhs.value_.data.number_float < static_cast<float_type>(rhs.value_.data.number_integer));
        }

        return false;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs < basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) < rhs;
    }

    // lte functions

    friend inline bool operator<=(const basic_config& lhs, const basic_config& rhs)
    {
        return !(rhs < lhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs <= basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) <= rhs;
    }

    // gt functions

    friend inline bool operator>(const basic_config& lhs, const basic_config& rhs)
    {
        return rhs < lhs;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs > basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) > rhs;
    }

    // gte functions

    friend inline bool operator>=(const basic_config& lhs, const basic_config& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs >= basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) >= rhs;
    }

public:
    const value_type& raw_value() const
    {
        return value_;
    }

    value_type& raw_value()
    {
        return value_;
    }

private:
    value_type value_;
};

}  // namespace configor
