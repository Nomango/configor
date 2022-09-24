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
#include "configor_value.hpp"

#include <algorithm>    // std::for_each, std::all_of
#include <sstream>      // std::stringstream
#include <type_traits>  // std::enable_if, std::is_same, std::is_integral, std::is_floating_point
#include <utility>      // std::forward, std::declval

namespace configor
{

template <typename _Args>
class basic_value
{
    friend struct detail::iterator<basic_value>;
    friend struct detail::iterator<const basic_value>;

public:
    template <typename _Ty>
    using allocator_type = typename _Args::template allocator_type<_Ty>;
    using boolean_type   = typename _Args::boolean_type;
    using integer_type   = typename _Args::integer_type;
    using float_type     = typename _Args::float_type;
    using char_type      = typename _Args::char_type;
    using string_type =
        typename _Args::template string_type<char_type, std::char_traits<char_type>, allocator_type<char_type>>;
    using array_type  = typename _Args::template array_type<basic_value, allocator_type<basic_value>>;
    using object_type = typename _Args::template object_type<string_type, basic_value, std::less<string_type>,
                                                             allocator_type<std::pair<const string_type, basic_value>>>;

    using value_type = detail::config_value<basic_value>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using iterator               = detail::iterator<basic_value>;
    using const_iterator         = detail::iterator<const basic_value>;
    using reverse_iterator       = detail::reverse_iterator<iterator>;
    using const_reverse_iterator = detail::reverse_iterator<const_iterator>;

    template <typename _Ty>
    using binder_type = typename _Args::template binder_type<_Ty>;

public:
    basic_value(std::nullptr_t = nullptr) {}

    basic_value(const config_value_type type)
        : value_(type)
    {
    }

    basic_value(const basic_value& other)
        : value_(other.value_)
    {
    }

    basic_value(basic_value&& other) noexcept
        : value_(std::move(other.value_))
    {
        other.value_.type        = config_value_type::null;
        other.value_.data.object = nullptr;
    }

    template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
              typename std::enable_if<!is_config<_UTy>::value && detail::has_to_config<basic_value, _UTy>::value,
                                      int>::type = 0>
    basic_value(_CompatibleTy&& value)
    {
        binder_type<_UTy>::to_config(*this, std::forward<_CompatibleTy>(value));
    }

    static basic_value object(std::initializer_list<std::pair<string_type, basic_value>> list)
    {
        basic_value v{ config_value_type::object };
        std::for_each(list.begin(), list.end(),
                      [&](const auto& pair) { v.value_.data.object->emplace(pair.first, pair.second); });
        return v;
    }

    static basic_value array(std::initializer_list<basic_value> list)
    {
        basic_value v{ config_value_type::array };
        v.value_.data.vector->reserve(list.size());
        v.value_.data.vector->assign(list.begin(), list.end());
        return v;
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

    inline void swap(basic_value& rhs)
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

    inline void push_back(basic_value&& config)
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

    inline basic_value& operator+=(basic_value&& config)
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
    // get pointer value

    inline boolean_type* do_get_ptr(boolean_type*) noexcept
    {
        return is_bool() ? &value_.data.boolean : nullptr;
    }

    inline const boolean_type* do_get_ptr(const boolean_type*) const noexcept
    {
        return is_bool() ? &value_.data.boolean : nullptr;
    }

    inline integer_type* do_get_ptr(integer_type*) noexcept
    {
        return is_integer() ? &value_.data.number_integer : nullptr;
    }

    inline const integer_type* do_get_ptr(const integer_type*) const noexcept
    {
        return is_integer() ? &value_.data.number_integer : nullptr;
    }

    inline float_type* do_get_ptr(float_type*) noexcept
    {
        return is_float() ? &value_.data.number_float : nullptr;
    }

    inline const float_type* do_get_ptr(const float_type*) const noexcept
    {
        return is_float() ? &value_.data.number_float : nullptr;
    }

    inline string_type* do_get_ptr(string_type*) noexcept
    {
        return is_string() ? value_.data.string : nullptr;
    }

    inline const string_type* do_get_ptr(const string_type*) const noexcept
    {
        return is_string() ? value_.data.string : nullptr;
    }

    inline array_type* do_get_ptr(array_type*) noexcept
    {
        return is_array() ? value_.data.vector : nullptr;
    }

    inline const array_type* do_get_ptr(const array_type*) const noexcept
    {
        return is_array() ? value_.data.vector : nullptr;
    }

    inline object_type* do_get_ptr(object_type*) noexcept
    {
        return is_object() ? value_.data.object : nullptr;
    }

    inline const object_type* do_get_ptr(const object_type*) const noexcept
    {
        return is_object() ? value_.data.object : nullptr;
    }

public:
    // get_ptr

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto get_ptr() const -> decltype(std::declval<const basic_value&>().do_get_ptr(std::declval<_Ty>()))
    {
        return do_get_ptr(static_cast<_Ty>(nullptr));
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto get_ptr() -> decltype(std::declval<basic_value&>().do_get_ptr(std::declval<_Ty>()))
    {
        return do_get_ptr(static_cast<_Ty>(nullptr));
    }

private:
    // get reference value
    template <typename _RefTy, typename _ValTy, typename _PtrTy = typename std::add_pointer<_RefTy>::type>
    static auto do_get_ref(_ValTy& c) -> decltype(c.template get_ptr<_PtrTy>(), std::declval<_RefTy>())
    {
        auto* ptr = c.template get_ptr<_PtrTy>();
        if (ptr != nullptr)
        {
            return *ptr;
        }
        throw configor_type_error("incompatible reference type for get, actual type is " + std::string(c.type_name()));
    }

    // get value

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_value, _Ty>::value>::type>
    _Ty do_get(detail::priority<3>) const
    {
        return *this;
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto do_get(detail::priority<2>) const noexcept
        -> decltype(std::declval<const basic_value&>().do_get_ptr(std::declval<_Ty>()))
    {
        return do_get_ptr(static_cast<_Ty>(nullptr));
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_config<basic_value, _Ty>::value>::type>
    _Ty do_get(detail::priority<1>) const
    {
        return binder_type<_Ty>::from_config(*this);
    }

    template <typename _Ty,
              typename = typename std::enable_if<std::is_default_constructible<_Ty>::value
                                                 && detail::has_from_config<basic_value, _Ty>::value>::type>
    _Ty do_get(detail::priority<0>) const
    {
        _Ty value{};
        binder_type<_Ty>::from_config(*this, value);
        return value;
    }

    // get value by reference

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_value, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<3>) const
    {
        return (value = *this);
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto do_get(_Ty& value, detail::priority<2>) const noexcept
        -> decltype(std::declval<const basic_value&>().do_get_ptr(std::declval<_Ty>()))
    {
        return (value = do_get_ptr(static_cast<_Ty>(nullptr)));
    }

    template <typename _Ty, typename = typename std::enable_if<detail::has_from_config<basic_value, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<1>) const
    {
        binder_type<_Ty>::from_config(*this, value);
        return value;
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_config<basic_value, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<0>) const
    {
        value = binder_type<_Ty>::from_config(*this);
        return value;
    }

public:
    // get

    template <typename _Ty, typename _UTy = typename detail::remove_cvref<_Ty>::type,
              typename = typename std::enable_if<!std::is_reference<_Ty>::value>::type>
    auto get() const -> decltype(std::declval<const basic_value&>().template do_get<_UTy>(detail::priority<3>{}))
    {
        return do_get<_UTy>(detail::priority<3>{});
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto get() -> decltype(std::declval<basic_value&>().template get_ptr<_Ty>())
    {
        return get_ptr<_Ty>();
    }

    template <typename _Ty, typename std::enable_if<std::is_reference<_Ty>::value, int>::type = 0>
    auto get() -> decltype(basic_value::template do_get_ref<_Ty>(std::declval<basic_value&>()))
    {
        return do_get_ref<_Ty>(*this);
    }

    template <typename _Ty,
              typename std::enable_if<std::is_reference<_Ty>::value
                                          && std::is_const<typename std::remove_reference<_Ty>::type>::value,
                                      int>::type = 0>
    auto get() const -> decltype(basic_value::template do_get_ref<_Ty>(std::declval<const basic_value&>()))
    {
        return do_get_ref<_Ty>(*this);
    }

    template <typename _Ty>
    auto get(_Ty& value) const
        -> decltype(std::declval<const basic_value&>().template do_get<_Ty>(std::declval<_Ty&>(),
                                                                            detail::priority<3>{}),
                    bool{})
    {
        try
        {
            do_get<_Ty>(value, detail::priority<3>{});
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    template <typename _Ty, typename = typename std::enable_if<detail::is_configor_getable<basic_value, _Ty>::value
                                                               && !std::is_pointer<_Ty>::value
                                                               && !std::is_reference<_Ty>::value>::type>
    inline operator _Ty() const
    {
        return get<_Ty>();
    }

public:
    // operator= functions

    inline basic_value& operator=(basic_value rhs)
    {
        swap(rhs);
        return (*this);
    }

    // operator[] functions

    inline basic_value& operator[](const size_type index)
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
                                       basic_value());
        }
        return (*value_.data.vector)[index];
    }

    inline basic_value& operator[](const size_type index) const
    {
        return at(index);
    }

    inline basic_value& operator[](const typename object_type::key_type& key)
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

    inline basic_value& operator[](const typename object_type::key_type& key) const
    {
        return at(key);
    }

    inline basic_value& at(const size_type index) const
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
    inline basic_value& operator[](_CharTy* key)
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
    inline basic_value& operator[](_CharTy* key) const
    {
        return at(key);
    }

    inline basic_value& at(const typename object_type::key_type& key) const
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
    inline basic_value& at(_CharTy* key) const
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
    // eq functions

    friend inline bool operator==(const basic_value& lhs, const basic_value& rhs)
    {
        return lhs.value_ == rhs.value_;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(const basic_value& lhs, _ScalarTy rhs)
    {
        return lhs == basic_value(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(_ScalarTy lhs, const basic_value& rhs)
    {
        return basic_value(lhs) == rhs;
    }

    // ne functions

    friend inline bool operator!=(const basic_value& lhs, const basic_value& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(const basic_value& lhs, _ScalarTy rhs)
    {
        return lhs != basic_value(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(_ScalarTy lhs, const basic_value& rhs)
    {
        return basic_value(lhs) != rhs;
    }

    // lt functions

    friend bool operator<(const basic_value& lhs, const basic_value& rhs)
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
    friend inline bool operator<(const basic_value& lhs, _ScalarTy rhs)
    {
        return lhs < basic_value(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(_ScalarTy lhs, const basic_value& rhs)
    {
        return basic_value(lhs) < rhs;
    }

    // lte functions

    friend inline bool operator<=(const basic_value& lhs, const basic_value& rhs)
    {
        return !(rhs < lhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(const basic_value& lhs, _ScalarTy rhs)
    {
        return lhs <= basic_value(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(_ScalarTy lhs, const basic_value& rhs)
    {
        return basic_value(lhs) <= rhs;
    }

    // gt functions

    friend inline bool operator>(const basic_value& lhs, const basic_value& rhs)
    {
        return rhs < lhs;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(const basic_value& lhs, _ScalarTy rhs)
    {
        return lhs > basic_value(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(_ScalarTy lhs, const basic_value& rhs)
    {
        return basic_value(lhs) > rhs;
    }

    // gte functions

    friend inline bool operator>=(const basic_value& lhs, const basic_value& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(const basic_value& lhs, _ScalarTy rhs)
    {
        return lhs >= basic_value(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(_ScalarTy lhs, const basic_value& rhs)
    {
        return basic_value(lhs) >= rhs;
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

namespace detail
{
template <typename _ValTy>
class value_maker
{
public:
    using value_type = _ValTy;

    struct object
    {
        using pair_type = std::pair<typename value_type::string_type, value_type>;

        object(std::initializer_list<pair_type> list)
            : list_(list)
        {
        }

        inline operator value_type() const
        {
            return value_type::object(list_);
        }

    private:
        std::initializer_list<pair_type> list_;
    };

    struct array
    {
        array(std::initializer_list<value_type> list)
            : list_(list)
        {
        }

        inline operator value_type() const
        {
            return value_type::array(list_);
        }

    private:
        std::initializer_list<value_type> list_;
    };
};
}  // namespace detail

}  // namespace configor
