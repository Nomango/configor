// Copyright (c) 2018-2020 jsonxx - Nomango
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
#include "json_conversion.hpp"
#include "json_declare.hpp"
#include "json_iterator.hpp"
#include "json_parser.hpp"
#include "json_serializer.hpp"
#include "json_value.hpp"

#include <algorithm>    // std::for_each, std::all_of
#include <type_traits>  // std::enable_if, std::is_same, std::is_integral, std::is_floating_point
#include <utility>      // std::forward, std::declval

namespace jsonxx
{

DECLARE_BASIC_JSON_TEMPLATE
class basic_json
{
    friend struct detail::iterator<basic_json>;
    friend struct detail::iterator<const basic_json>;
    friend struct detail::json_serializer<basic_json>;
    friend struct detail::json_parser<basic_json>;

public:
    template <typename _Ty>
    using allocator_type  = _Allocator<_Ty>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using string_type     = _StringTy;
    using char_type       = typename _StringTy::value_type;
    using integer_type    = _IntegerTy;
    using float_type      = _FloatTy;
    using boolean_type    = _BooleanTy;
    using array_type      = _ArrayTy<basic_json, allocator_type<basic_json>>;
    using object_type     = _ObjectTy<string_type, basic_json, std::less<string_type>, allocator_type<std::pair<const string_type, basic_json>>>;

    using iterator               = detail::iterator<basic_json>;
    using const_iterator         = detail::iterator<const basic_json>;
    using reverse_iterator       = detail::reverse_iterator<iterator>;
    using const_reverse_iterator = detail::reverse_iterator<const_iterator>;

    using encoding_type = _Encoding<char_type>;

    using dump_args  = detail::serializer_args<basic_json>;
    using parse_args = detail::parser_args<basic_json>;

public:
    basic_json(std::nullptr_t = nullptr) {}

    basic_json(const json_type type)
        : value_(type)
    {
    }

    basic_json(const basic_json& other)
        : value_(other.value_)
    {
    }

    basic_json(basic_json&& other) noexcept
        : value_(std::move(other.value_))
    {
        // invalidate payload
        other.value_.type        = json_type::null;
        other.value_.data.object = nullptr;
    }

    basic_json(const std::initializer_list<basic_json>& init_list, json_type exact_type = json_type::null)
    {
        bool is_an_object = std::all_of(init_list.begin(), init_list.end(), [](const basic_json& json) { return (json.is_array() && json.size() == 2 && json[0].is_string()); });

        if (exact_type != json_type::object && exact_type != json_type::array)
        {
            exact_type = is_an_object ? json_type::object : json_type::array;
        }

        if (exact_type == json_type::object)
        {
            if (!is_an_object)
                throw json_type_error("initializer_list is not object type");

            value_ = json_type::object;
            std::for_each(init_list.begin(), init_list.end(),
                          [this](const basic_json& json) { value_.data.object->emplace(*((*json.value_.data.vector)[0].value_.data.string), (*json.value_.data.vector)[1]); });
        }
        else
        {
            value_ = json_type::array;
            value_.data.vector->reserve(init_list.size());
            value_.data.vector->assign(init_list.begin(), init_list.end());
        }
    }

    template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
              typename = typename std::enable_if<!std::is_same<basic_json, _UTy>::value && detail::has_to_json<basic_json, _UTy>::value>::type>
    basic_json(_CompatibleTy&& value)
    {
        json_bind<_UTy>::to_json(*this, std::forward<_CompatibleTy>(value));
    }

    static inline basic_json object(const std::initializer_list<basic_json>& init_list)
    {
        return basic_json(init_list, json_type::object);
    }

    static inline basic_json array(const std::initializer_list<basic_json>& init_list)
    {
        return basic_json(init_list, json_type::array);
    }

    inline bool is_object() const
    {
        return value_.type == json_type::object;
    }

    inline bool is_array() const
    {
        return value_.type == json_type::array;
    }

    inline bool is_string() const
    {
        return value_.type == json_type::string;
    }

    inline bool is_bool() const
    {
        return value_.type == json_type::boolean;
    }

    inline bool is_integer() const
    {
        return value_.type == json_type::number_integer;
    }

    inline bool is_float() const
    {
        return value_.type == json_type::number_float;
    }

    inline bool is_number() const
    {
        return is_integer() || is_float();
    }

    inline bool is_null() const
    {
        return value_.type == json_type::null;
    }

    inline json_type type() const
    {
        return value_.type;
    }

    inline const char* type_name() const
    {
        return detail::to_string(type());
    }

    inline void swap(basic_json& rhs)
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
        case json_type::null:
            return 0;
        case json_type::array:
            return value_.data.vector->size();
        case json_type::object:
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

    template <typename _Kty>
    inline const_iterator find(_Kty&& key) const
    {
        if (is_object())
        {
            const_iterator iter(this);
            iter.object_it_ = value_.data.object->find(std::forward<_Kty>(key));
            return iter;
        }
        return cend();
    }

    template <typename _Kty>
    inline size_type count(_Kty&& key) const
    {
        return is_object() ? value_.data.object->count(std::forward<_Kty>(key)) : 0;
    }

    inline size_type erase(const typename object_type::key_type& key)
    {
        if (!is_object())
        {
            throw json_invalid_key("cannot use erase() with non-object value");
        }
        return value_.data.object->erase(key);
    }

    inline void erase(const size_type index)
    {
        if (!is_array())
        {
            throw json_invalid_key("cannot use erase() with non-array value");
        }
        value_.data.vector->erase(value_.data.vector->begin() + static_cast<difference_type>(index));
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value || std::is_same<_IterTy, const_iterator>::value>::type>
    inline _IterTy erase(_IterTy pos)
    {
        _IterTy result = end();

        switch (type())
        {
        case json_type::object:
        {
            result.it_.object_iter = value_.data.object->erase(pos.it_.object_iter);
            break;
        }

        case json_type::array:
        {
            result.it_.array_iter = value_.data.vector->erase(pos.it_.array_iter);
            break;
        }

        default:
            throw json_invalid_iterator("cannot use erase() with non-object & non-array value");
        }

        return result;
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value || std::is_same<_IterTy, const_iterator>::value>::type>
    inline _IterTy erase(_IterTy first, _IterTy last)
    {
        _IterTy result = end();

        switch (type())
        {
        case json_type::object:
        {
            result.it_.object_iter = value_.data.object->erase(first.it_.object_iter, last.it_.object_iter);
            break;
        }

        case json_type::array:
        {
            result.it_.array_iter = value_.data.vector->erase(first.it_.array_iter, last.it_.array_iter);
            break;
        }

        default:
            throw json_invalid_iterator("cannot use erase() with non-object & non-array value");
        }

        return result;
    }

    inline void push_back(basic_json&& json)
    {
        if (!is_null() && !is_array())
        {
            throw json_type_error("cannot use push_back() with non-array value");
        }

        if (is_null())
        {
            value_ = json_type::array;
        }

        value_.data.vector->push_back(std::move(json));
    }

    inline basic_json& operator+=(basic_json&& json)
    {
        push_back(std::move(json));
        return (*this);
    }

    inline void clear()
    {
        switch (type())
        {
        case json_type::number_integer:
        {
            value_.data.number_integer = 0;
            break;
        }

        case json_type::number_float:
        {
            value_.data.number_float = static_cast<float_type>(0.0);
            break;
        }

        case json_type::boolean:
        {
            value_.data.boolean = false;
            break;
        }

        case json_type::string:
        {
            value_.data.string->clear();
            break;
        }

        case json_type::array:
        {
            value_.data.vector->clear();
            break;
        }

        case json_type::object:
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

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_json, _Ty>::value>::type>
    inline _Ty do_get(detail::priority<2>) const
    {
        return *this;
    }

    template <typename _Ty, typename = typename std::enable_if<detail::has_non_default_from_json<basic_json, _Ty>::value>::type>
    inline _Ty do_get(detail::priority<1>) const
    {
        return json_bind<_Ty>::from_json(*this);
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_default_constructible<_Ty>::value && detail::has_default_from_json<basic_json, _Ty>::value>::type>
    inline _Ty do_get(detail::priority<0>) const
    {
        _Ty value{};
        json_bind<_Ty>::from_json(*this, value);
        return value;
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_json, _Ty>::value>::type>
    inline _Ty& do_get(_Ty& value, detail::priority<2>) const
    {
        return (value = *this);
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_default_constructible<_Ty>::value && detail::has_default_from_json<basic_json, _Ty>::value>::type>
    inline _Ty& do_get(_Ty& value, detail::priority<1>) const
    {
        json_bind<_Ty>::from_json(*this, value);
        return value;
    }

    template <typename _Ty, typename = typename std::enable_if<detail::has_non_default_from_json<basic_json, _Ty>::value>::type>
    inline _Ty& do_get(_Ty& value, detail::priority<0>) const
    {
        value = json_bind<_Ty>::from_json(*this);
        return value;
    }

public:
    template <typename _Ty, typename _UTy = typename detail::remove_cvref<_Ty>::type>
    inline auto get() const -> decltype(std::declval<const basic_json&>().template do_get<_UTy>(detail::priority<2>{}))
    {
        return do_get<_UTy>(detail::priority<2>{});
    }

    template <typename _Ty>
    inline auto get(_Ty& value) const -> decltype(std::declval<const basic_json&>().template do_get<_Ty>(std::declval<_Ty&>(), detail::priority<2>{}))
    {
        return do_get<_Ty>(value, detail::priority<2>{});
    }

    template <typename _Ty>
    auto try_get(_Ty& value) const -> typename std::enable_if<!std::is_void<decltype(std::declval<const basic_json&>().template get<_Ty>(std::declval<_Ty&>()))>::value, bool>::type
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
        case json_type::number_integer:
            return value_.data.number_integer != 0;
        case json_type::number_float:
            return value_.data.number_float != 0;
        case json_type::string:
        case json_type::array:
        case json_type::object:
            return !empty();
        case json_type::boolean:
            return value_.data.boolean;
        case json_type::null:
            break;
        }
        return false;
    }

    integer_type as_integer() const
    {
        switch (type())
        {
        case json_type::number_integer:
            return value_.data.number_integer;
        case json_type::number_float:
            return static_cast<integer_type>(value_.data.number_float);
        case json_type::boolean:
            return value_.data.boolean ? integer_type(1) : integer_type(0);
        case json_type::string:
        case json_type::array:
        case json_type::object:
        case json_type::null:
            throw detail::make_conversion_error(type(), json_type::number_integer);
        }
        return 0;
    }

    float_type as_float() const
    {
        switch (type())
        {
        case json_type::number_integer:
            return static_cast<float_type>(value_.data.number_integer);
        case json_type::number_float:
            return value_.data.number_float;
        case json_type::boolean:
            return value_.data.boolean ? float_type(1) : float_type(0);
        case json_type::string:
        case json_type::array:
        case json_type::object:
        case json_type::null:
            throw detail::make_conversion_error(type(), json_type::number_integer);
        }
        return 0;
    }

    string_type as_string() const
    {
        switch (type())
        {
        case json_type::string:
            return *value_.data.string;
        case json_type::number_integer:
        case json_type::number_float:
        case json_type::boolean:
            return dump();
        case json_type::array:
        case json_type::object:
            throw detail::make_conversion_error(type(), json_type::number_integer);
        case json_type::null:
            break;
        }
        return string_type{};
    }

public:
    // operator= functions

    inline basic_json& operator=(basic_json rhs)
    {
        swap(rhs);
        return (*this);
    }

public:
    // operator[] functions

    inline basic_json& operator[](const size_type index)
    {
        if (is_null())
        {
            value_ = json_type::array;
        }

        if (!is_array())
        {
            throw json_invalid_key("operator[] called on a non-array object");
        }

        if (index >= value_.data.vector->size())
        {
            value_.data.vector->insert(value_.data.vector->end(), index - value_.data.vector->size() + 1, basic_json());
        }
        return (*value_.data.vector)[index];
    }

    inline basic_json& operator[](const size_type index) const
    {
        return at(index);
    }

    inline basic_json& operator[](const typename object_type::key_type& key)
    {
        if (is_null())
        {
            value_ = json_type::object;
        }

        if (!is_object())
        {
            throw json_invalid_key("operator[] called on a non-object type");
        }
        return (*value_.data.object)[key];
    }

    inline basic_json& operator[](const typename object_type::key_type& key) const
    {
        return at(key);
    }

    inline basic_json& at(const size_type index) const
    {
        if (!is_array())
        {
            throw json_invalid_key("operator[] called on a non-array type");
        }

        if (index >= value_.data.vector->size())
        {
            throw std::out_of_range("operator[] index out of range");
        }
        return (*value_.data.vector)[index];
    }

    template <typename _CharTy>
    inline basic_json& operator[](_CharTy* key)
    {
        if (is_null())
        {
            value_ = json_type::object;
        }

        if (!is_object())
        {
            throw json_invalid_key("operator[] called on a non-object object");
        }
        return (*value_.data.object)[key];
    }

    template <typename _CharTy>
    inline basic_json& operator[](_CharTy* key) const
    {
        return at(key);
    }

    inline basic_json& at(const typename object_type::key_type& key) const
    {
        if (!is_object())
        {
            throw json_invalid_key("operator[] called on a non-object object");
        }

        auto iter = value_.data.object->find(key);
        if (iter == value_.data.object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

    template <typename _CharTy>
    inline basic_json& at(_CharTy* key) const
    {
        if (!is_object())
        {
            throw json_invalid_key("operator[] called on a non-object object");
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

    template <typename _Ty, typename = typename std::enable_if<!is_json<_Ty>::value && detail::is_json_getable<basic_json, _Ty>::value && !std::is_pointer<_Ty>::value>::type>
    inline operator _Ty() const
    {
        return get<_Ty>();
    }

    // to_json functions

    template <typename _BoolTy, typename = typename std::enable_if<std::is_same<_BoolTy, boolean_type>::value>::type>
    friend inline void to_json(basic_json& j, _BoolTy value)
    {
        j.value_.type         = json_type::boolean;
        j.value_.data.boolean = value;
    }

    friend inline void to_json(basic_json& j, integer_type value)
    {
        j.value_.type                = json_type::number_integer;
        j.value_.data.number_integer = value;
    }

    template <typename _IntegerUTy, typename std::enable_if<!std::is_same<_IntegerUTy, boolean_type>::value && std::is_integral<_IntegerUTy>::value, int>::type = 0>
    friend inline void to_json(basic_json& j, _IntegerUTy value)
    {
        j.value_.type                = json_type::number_integer;
        j.value_.data.number_integer = static_cast<integer_type>(value);
    }

    friend inline void to_json(basic_json& j, float_type value)
    {
        j.value_.type              = json_type::number_float;
        j.value_.data.number_float = value;
    }

    template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
    friend inline void to_json(basic_json& j, _FloatingTy value)
    {
        j.value_.type              = json_type::number_float;
        j.value_.data.number_float = static_cast<float_type>(value);
    }

    friend inline void to_json(basic_json& j, const string_type& value)
    {
        j.value_.type        = json_type::string;
        j.value_.data.string = j.value_.template create<string_type>(value);
    }

    friend inline void to_json(basic_json& j, const array_type& value)
    {
        j.value_.type        = json_type::array;
        j.value_.data.vector = j.value_.template create<array_type>(value);
    }

    friend inline void to_json(basic_json& j, const object_type& value)
    {
        j.value_.type        = json_type::object;
        j.value_.data.object = j.value_.template create<object_type>(value);
    }

    // from_json functions

    template <typename _BoolTy, typename = typename std::enable_if<std::is_same<_BoolTy, boolean_type>::value>::type>
    friend inline void from_json(const basic_json& j, _BoolTy& value)
    {
        if (!j.is_bool())
            throw detail::make_conversion_error(j.type(), json_type::boolean);
        value = j.value_.data.boolean;
    }

    friend inline void from_json(const basic_json& j, integer_type& value)
    {
        if (!j.is_integer())
            throw detail::make_conversion_error(j.type(), json_type::number_integer);
        value = j.value_.data.number_integer;
    }

    template <typename _IntegerUTy, typename std::enable_if<!std::is_same<_IntegerUTy, boolean_type>::value && std::is_integral<_IntegerUTy>::value, int>::type = 0>
    friend inline void from_json(const basic_json& j, _IntegerUTy& value)
    {
        if (!j.is_integer())
            throw detail::make_conversion_error(j.type(), json_type::number_integer);
        value = static_cast<_IntegerUTy>(j.value_.data.number_integer);
    }

    friend inline void from_json(const basic_json& j, float_type& value)
    {
        if (!j.is_float())
            throw detail::make_conversion_error(j.type(), json_type::number_float);
        value = j.value_.data.number_float;
    }

    template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
    friend inline void from_json(const basic_json& j, _FloatingTy& value)
    {
        if (!j.is_float())
            throw detail::make_conversion_error(j.type(), json_type::number_float);
        value = static_cast<_FloatingTy>(j.value_.data.number_float);
    }

    friend inline void from_json(const basic_json& j, string_type& value)
    {
        if (!j.is_string())
            throw detail::make_conversion_error(j.type(), json_type::string);
        value = *j.value_.data.string;
    }

    friend inline void from_json(const basic_json& j, array_type& value)
    {
        if (!j.is_array())
            throw detail::make_conversion_error(j.type(), json_type::array);
        value.assign((*j.value_.data.vector).begin(), (*j.value_.data.vector).end());
    }

    friend inline void from_json(const basic_json& j, object_type& value)
    {
        if (!j.is_object())
            throw detail::make_conversion_error(j.type(), json_type::object);
        value = j.value_.data.object;
    }

public:
    // dumps functions

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const basic_json& j)
    {
        dump_args args;
        args.indent      = static_cast<unsigned int>(os.width());
        args.indent_char = os.fill();
        args.precision   = static_cast<int>(os.precision());

        os.width(0);

        j.dump(os, args);
        return os;
    }

    string_type dump(const dump_args& args = dump_args{}, error_handler* eh = nullptr) const
    {
        string_type                               result;
        detail::fast_string_ostreambuf<char_type> buf{ result };
        std::basic_ostream<char_type>             os{ &buf };
        this->dump(os, args, eh);
        return result;
    }

    string_type dump(unsigned int indent, char_type indent_char = ' ', bool escape_unicode = false, error_handler* eh = nullptr) const
    {
        dump_args args;
        args.indent         = indent;
        args.indent_char    = indent_char;
        args.escape_unicode = escape_unicode;
        return this->dump(args, eh);
    }

    void dump(std::basic_ostream<char_type>& os, const dump_args& args = dump_args{}, error_handler* eh = nullptr) const
    {
        try
        {
            detail::json_serializer<basic_json>{ os, args }.dump(*this);
        }
        catch (...)
        {
            if (eh)
                eh->handle(std::current_exception());
            else
                throw;
        }
    }

public:
    // parse functions

    friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& is, basic_json& j)
    {
        basic_json::parse(j, is);
        return is;
    }

    static inline basic_json parse(const string_type& str, const parse_args& args = parse_args{}, error_handler* eh = nullptr)
    {
        detail::fast_string_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return basic_json::parse(is, args, eh);
    }

    static inline basic_json parse(const char_type* str, const parse_args& args = parse_args{}, error_handler* eh = nullptr)
    {
        detail::fast_buffer_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return basic_json::parse(is, args, eh);
    }

    static inline basic_json parse(std::FILE* file, const parse_args& args = parse_args{}, error_handler* eh = nullptr)
    {
        detail::fast_cfile_istreambuf<char_type> buf{ file };
        std::basic_istream<char_type>            is{ &buf };
        return basic_json::parse(is, args, eh);
    }

    static inline basic_json parse(std::basic_istream<char_type>& is, const parse_args& args = parse_args{}, error_handler* eh = nullptr)
    {
        basic_json result;
        basic_json::parse(result, is, args, eh);
        return result;
    }

    static inline void parse(basic_json& j, std::basic_istream<char_type>& is, const parse_args& args = parse_args{}, error_handler* eh = nullptr)
    {
        try
        {
            detail::json_parser<basic_json>{ is, args }.parse(j);
            if (args.check_document && !j.is_array() && !j.is_object())
            {
                std::string name = j.type_name();
                throw json_deserialization_error("invalid document type '" + name + "'");
            }
        }
        catch (...)
        {
            if (eh)
                eh->handle(std::current_exception());
            else
                throw;
        }
    }

public:
    // eq functions

    friend inline bool operator==(const basic_json& lhs, const basic_json& rhs)
    {
        return lhs.value_ == rhs.value_;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(const basic_json& lhs, _ScalarTy rhs)
    {
        return lhs == basic_json(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(_ScalarTy lhs, const basic_json& rhs)
    {
        return basic_json(lhs) == rhs;
    }

    // ne functions

    friend inline bool operator!=(const basic_json& lhs, const basic_json& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(const basic_json& lhs, _ScalarTy rhs)
    {
        return lhs != basic_json(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(_ScalarTy lhs, const basic_json& rhs)
    {
        return basic_json(lhs) != rhs;
    }

    // lt functions

    friend bool operator<(const basic_json& lhs, const basic_json& rhs)
    {
        const auto lhs_type = lhs.type();
        const auto rhs_type = rhs.type();

        if (lhs_type == rhs_type)
        {
            switch (lhs_type)
            {
            case json_type::array:
                return (*lhs.value_.data.vector) < (*rhs.value_.data.vector);

            case json_type::object:
                return (*lhs.value_.data.object) < (*rhs.value_.data.object);

            case json_type::null:
                return false;

            case json_type::string:
                return (*lhs.value_.data.string) < (*rhs.value_.data.string);

            case json_type::boolean:
                return (lhs.value_.data.boolean < rhs.value_.data.boolean);

            case json_type::number_integer:
                return (lhs.value_.data.number_integer < rhs.value_.data.number_integer);

            case json_type::number_float:
                return (lhs.value_.data.number_float < rhs.value_.data.number_float);

            default:
                return false;
            }
        }
        else if (lhs_type == json_type::number_integer && rhs_type == json_type::number_float)
        {
            return (static_cast<float_type>(lhs.value_.data.number_integer) < rhs.value_.data.number_float);
        }
        else if (lhs_type == json_type::number_float && rhs_type == json_type::number_integer)
        {
            return (lhs.value_.data.number_float < static_cast<float_type>(rhs.value_.data.number_integer));
        }

        return false;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(const basic_json& lhs, _ScalarTy rhs)
    {
        return lhs < basic_json(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(_ScalarTy lhs, const basic_json& rhs)
    {
        return basic_json(lhs) < rhs;
    }

    // lte functions

    friend inline bool operator<=(const basic_json& lhs, const basic_json& rhs)
    {
        return !(rhs < lhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(const basic_json& lhs, _ScalarTy rhs)
    {
        return lhs <= basic_json(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(_ScalarTy lhs, const basic_json& rhs)
    {
        return basic_json(lhs) <= rhs;
    }

    // gt functions

    friend inline bool operator>(const basic_json& lhs, const basic_json& rhs)
    {
        return rhs < lhs;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(const basic_json& lhs, _ScalarTy rhs)
    {
        return lhs > basic_json(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(_ScalarTy lhs, const basic_json& rhs)
    {
        return basic_json(lhs) > rhs;
    }

    // gte functions

    friend inline bool operator>=(const basic_json& lhs, const basic_json& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(const basic_json& lhs, _ScalarTy rhs)
    {
        return lhs >= basic_json(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(_ScalarTy lhs, const basic_json& rhs)
    {
        return basic_json(lhs) >= rhs;
    }

private:
    detail::json_value<basic_json> value_;
};

}  // namespace jsonxx
