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
#include "configor_iterator.hpp"

#include <algorithm>    // std::for_each, std::all_of
#include <cmath>        // std::fabs
#include <limits>       // std::numeric_limits
#include <memory>       // std::allocator_traits
#include <sstream>      // std::stringstream
#include <type_traits>  // std::enable_if, std::is_same, std::is_integral, std::is_floating_point
#include <utility>      // std::swap, std::forward, std::declval

namespace configor
{

namespace detail
{
template <typename _ValTy>
class value_constructor;
}  // namespace detail

template <typename _Args>
class basic_value final : public value_constant
{
    friend detail::value_constructor<basic_value>;

public:
    template <typename _Ty>
    using allocator_type = typename _Args::template allocator_type<_Ty>;

    using boolean_type = typename _Args::boolean_type;
    using integer_type = typename _Args::integer_type;
    using float_type   = typename _Args::float_type;
    using char_type    = typename _Args::char_type;
    using string_type =
        typename _Args::template string_type<char_type, std::char_traits<char_type>, allocator_type<char_type>>;
    using array_type  = typename _Args::template array_type<basic_value, allocator_type<basic_value>>;
    using object_type = typename _Args::template object_type<string_type, basic_value, std::less<string_type>,
                                                             allocator_type<std::pair<const string_type, basic_value>>>;

    template <typename _Ty>
    using binder_type = typename _Args::template binder_type<_Ty>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using iterator               = detail::iterator<basic_value>;
    using const_iterator         = detail::iterator<const basic_value>;
    using reverse_iterator       = detail::reverse_iterator<iterator>;
    using const_reverse_iterator = detail::reverse_iterator<const_iterator>;

    union data_type
    {
        boolean_type boolean;
        integer_type integer;
        float_type   floating;
        string_type* string;
        object_type* object;
        array_type*  vector;
    };

    basic_value(std::nullptr_t = nullptr)
        : type_{ value_constant::null }
        , data_{}
    {
    }

    basic_value(const value_constant::type t)
        : type_{ t }
        , data_{}
    {
        using accessor = detail::value_constructor<basic_value>;
        switch (t)
        {
        case value_constant::object:
            accessor::template construct<value_constant::object>(*this);
            break;
        case value_constant::array:
            accessor::template construct<value_constant::array>(*this);
            break;
        case value_constant::string:
            accessor::template construct<value_constant::string>(*this);
            break;
        case value_constant::integer:
            accessor::template construct<value_constant::integer>(*this, integer_type(0));
            break;
        case value_constant::floating:
            accessor::template construct<value_constant::floating>(*this, float_type(0.0));
            break;
        case value_constant::boolean:
            accessor::template construct<value_constant::boolean>(*this, boolean_type(false));
            break;
        default:
            break;
        }
    }

    basic_value(const basic_value& other)
        : type_{ other.type() }
        , data_{}
    {
        using accessor = detail::value_constructor<basic_value>;
        switch (other.type())
        {
        case value_constant::object:
            accessor::template construct<value_constant::object>(*this, *other.data().object);
            break;
        case value_constant::array:
            accessor::template construct<value_constant::array>(*this, *other.data().vector);
            break;
        case value_constant::string:
            accessor::template construct<value_constant::string>(*this, *other.data().string);
            break;
        case value_constant::integer:
            accessor::template construct<value_constant::integer>(*this, other.data().integer);
            break;
        case value_constant::floating:
            accessor::template construct<value_constant::floating>(*this, other.data().floating);
            break;
        case value_constant::boolean:
            accessor::template construct<value_constant::boolean>(*this, other.data().boolean);
            break;
        default:
            break;
        }
    }

    basic_value(basic_value&& other) noexcept
        : type_{ other.type() }
        , data_{ other.data() }
    {
        other.type(value_constant::null);
        other.data().object = nullptr;
    }

    template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
              typename std::enable_if<!is_value<_UTy>::value && detail::has_to_value<basic_value, _UTy>::value,
                                      int>::type = 0>
    basic_value(_CompatibleTy&& value)
        : type_{ value_constant::null }
        , data_{}
    {
        binder_type<_UTy>::to_value(*this, std::forward<_CompatibleTy>(value));
    }

    ~basic_value()
    {
        using accessor = detail::value_constructor<basic_value>;
        accessor::destroy(*this);
    }

    const data_type& data() const
    {
        return data_;
    }

    data_type& data()
    {
        return data_;
    }

    inline value_constant::type type() const
    {
        return type_;
    }

private:
    inline void type(value_constant::type t)
    {
        type_ = t;
    }

public:
    inline bool is_object() const
    {
        return type() == value_constant::object;
    }

    inline bool is_array() const
    {
        return type() == value_constant::array;
    }

    inline bool is_string() const
    {
        return type() == value_constant::string;
    }

    inline bool is_bool() const
    {
        return type() == value_constant::boolean;
    }

    inline bool is_integer() const
    {
        return type() == value_constant::integer;
    }

    inline bool is_floating() const
    {
        return type() == value_constant::floating;
    }

    inline bool is_number() const
    {
        return is_integer() || is_floating();
    }

    inline bool is_null() const
    {
        return type() == value_constant::null;
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
        case value_constant::null:
            return 0;
        case value_constant::array:
            return data().vector->size();
        case value_constant::object:
            return data().object->size();
        default:
            return 1;
        }
    }

    bool empty() const
    {
        if (is_null())
            return true;

        if (is_object())
            return data().object->empty();

        if (is_array())
            return data().vector->empty();

        return false;
    }

    iterator find(const typename object_type::key_type& key)
    {
        if (is_object())
        {
            iterator iter(this);
            iter.object_it_ = data().object->find(key);
            return iter;
        }
        return end();
    }

    const_iterator find(const typename object_type::key_type& key) const
    {
        if (is_object())
        {
            const_iterator iter(this);
            iter.object_it_ = data().object->find(key);
            return iter;
        }
        return cend();
    }

    inline size_type count(const typename object_type::key_type& key) const
    {
        return is_object() ? data().object->count(key) : 0;
    }

    size_type erase(const typename object_type::key_type& key)
    {
        if (!is_object())
        {
            throw configor_invalid_key("cannot use erase() with non-object value");
        }
        return data().object->erase(key);
    }

    void erase(const size_type index)
    {
        if (!is_array())
        {
            throw configor_invalid_key("cannot use erase() with non-array value");
        }
        data().vector->erase(data().vector->begin() + static_cast<difference_type>(index));
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy pos)
    {
        _IterTy result = end();

        switch (type())
        {
        case value_constant::object:
        {
            result.object_it_ = data().object->erase(pos.object_it_);
            break;
        }

        case value_constant::array:
        {
            result.array_it_ = data().vector->erase(pos.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy first, _IterTy last)
    {
        _IterTy result = end();

        switch (type())
        {
        case value_constant::object:
        {
            result.object_it_ = data().object->erase(first.object_it_, last.object_it_);
            break;
        }

        case value_constant::array:
        {
            result.array_it_ = data().vector->erase(first.array_it_, last.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    void push_back(const basic_value& v)
    {
        emplace_back(v);
    }

    void push_back(basic_value&& v)
    {
        emplace_back(std::move(v));
    }

    template <typename... _Ty>
    void emplace_back(_Ty&&... v)
    {
        if (!is_null() && !is_array())
        {
            throw configor_type_error("use push_back() or emplace_back() with non-array value");
        }
        if (is_null())
        {
            *this = value_constant::array;
        }
        data().vector->emplace_back(std::forward<_Ty>(v)...);
    }

    void clear()
    {
        switch (type())
        {
        case value_constant::integer:
            data().integer = 0;
            break;
        case value_constant::floating:
            data().floating = static_cast<float_type>(0.0);
            break;
        case value_constant::boolean:
            data().boolean = false;
            break;
        case value_constant::string:
            data().string->clear();
            break;
        case value_constant::array:
            data().vector->clear();
            break;
        case value_constant::object:
            data().object->clear();
            break;
        default:
            break;
        }
    }

private:
    // get pointer value

    inline boolean_type* do_get_ptr(boolean_type*) noexcept
    {
        return is_bool() ? &data().boolean : nullptr;
    }

    inline const boolean_type* do_get_ptr(const boolean_type*) const noexcept
    {
        return is_bool() ? &data().boolean : nullptr;
    }

    inline integer_type* do_get_ptr(integer_type*) noexcept
    {
        return is_integer() ? &data().integer : nullptr;
    }

    inline const integer_type* do_get_ptr(const integer_type*) const noexcept
    {
        return is_integer() ? &data().integer : nullptr;
    }

    inline float_type* do_get_ptr(float_type*) noexcept
    {
        return is_floating() ? &data().floating : nullptr;
    }

    inline const float_type* do_get_ptr(const float_type*) const noexcept
    {
        return is_floating() ? &data().floating : nullptr;
    }

    inline string_type* do_get_ptr(string_type*) noexcept
    {
        return is_string() ? data().string : nullptr;
    }

    inline const string_type* do_get_ptr(const string_type*) const noexcept
    {
        return is_string() ? data().string : nullptr;
    }

    inline array_type* do_get_ptr(array_type*) noexcept
    {
        return is_array() ? data().vector : nullptr;
    }

    inline const array_type* do_get_ptr(const array_type*) const noexcept
    {
        return is_array() ? data().vector : nullptr;
    }

    inline object_type* do_get_ptr(object_type*) noexcept
    {
        return is_object() ? data().object : nullptr;
    }

    inline const object_type* do_get_ptr(const object_type*) const noexcept
    {
        return is_object() ? data().object : nullptr;
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
        throw configor_type_error(std::string("incompatible reference type for get, actual type is ")
                                  + to_string(c.type()));
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
              typename = typename std::enable_if<detail::has_non_default_from_value<basic_value, _Ty>::value>::type>
    _Ty do_get(detail::priority<1>) const
    {
        return binder_type<_Ty>::from_value(*this);
    }

    template <typename _Ty,
              typename = typename std::enable_if<std::is_default_constructible<_Ty>::value
                                                 && detail::has_from_value<basic_value, _Ty>::value>::type>
    _Ty do_get(detail::priority<0>) const
    {
        _Ty value{};
        binder_type<_Ty>::from_value(*this, value);
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

    template <typename _Ty, typename = typename std::enable_if<detail::has_from_value<basic_value, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<1>) const
    {
        binder_type<_Ty>::from_value(*this, value);
        return value;
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_value<basic_value, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<0>) const
    {
        value = binder_type<_Ty>::from_value(*this);
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

    template <typename _Ty, typename = typename std::enable_if<detail::is_value_getable<basic_value, _Ty>::value
                                                               && !std::is_pointer<_Ty>::value
                                                               && !std::is_reference<_Ty>::value>::type>
    inline operator _Ty() const
    {
        return get<_Ty>();
    }

public:
    // swap function
    inline void swap(basic_value& rhs)
    {
        std::swap(type_, rhs.type_);
        std::swap(data_, rhs.data_);
    }

    // operator= functions

    basic_value& operator=(const basic_value& rhs)
    {
        if (this != &rhs)
        {
            basic_value tmp{ rhs };
            this->swap(tmp);
        }
        return *this;
    }

    basic_value& operator=(basic_value&& rhs)
    {
        if (this != &rhs)
        {
            basic_value old{ std::move(*this) };
            this->swap(rhs);
        }
        return *this;
    }

    // operator[] functions

    basic_value& operator[](const size_type index)
    {
        if (is_null())
        {
            *this = value_constant::array;
        }

        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array object");
        }

        if (index >= data().vector->size())
        {
            data().vector->insert(data().vector->end(), index - data().vector->size() + 1, basic_value());
        }
        return (*data().vector)[index];
    }

    inline basic_value& operator[](const size_type index) const
    {
        return at(index);
    }

    basic_value& operator[](const typename object_type::key_type& key)
    {
        if (is_null())
        {
            *this = value_constant::object;
        }
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object type");
        }
        return (*data().object)[key];
    }

    inline basic_value& operator[](const typename object_type::key_type& key) const
    {
        return at(key);
    }

    basic_value& at(const size_type index) const
    {
        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array type");
        }
        if (index >= data().vector->size())
        {
            throw std::out_of_range("operator[] index out of range");
        }
        return (*data().vector)[index];
    }

    template <typename _CharTy>
    basic_value& operator[](_CharTy* key)
    {
        if (is_null())
        {
            *this = value_constant::object;
        }

        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }
        return (*data().object)[key];
    }

    template <typename _CharTy>
    inline basic_value& operator[](_CharTy* key) const
    {
        return at(key);
    }

    basic_value& at(const typename object_type::key_type& key) const
    {
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }

        auto iter = data().object->find(key);
        if (iter == data().object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

    template <typename _CharTy>
    basic_value& at(_CharTy* key) const
    {
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }

        auto iter = data().object->find(key);
        if (iter == data().object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

public:
    // eq functions

    friend bool operator==(const basic_value& lhs, const basic_value& rhs)
    {
        if (lhs.type() == rhs.type())
        {
            switch (lhs.type())
            {
            case value_constant::array:
                return (*lhs.data().vector == *rhs.data().vector);
            case value_constant::object:
                return (*lhs.data().object == *rhs.data().object);
            case value_constant::null:
                return true;
            case value_constant::string:
                return *lhs.data().string == *rhs.data().string;
            case value_constant::boolean:
                return lhs.data().boolean == rhs.data().boolean;
            case value_constant::integer:
                return lhs.data().integer == rhs.data().integer;
            case value_constant::floating:
                return lhs.data().floating == rhs.data().floating;
            default:
                return false;
            }
        }
        else if (lhs.type() == value_constant::integer && rhs.type() == value_constant::floating)
        {
            return static_cast<float_type>(lhs.data().integer) == rhs.data().floating;
        }
        else if (lhs.type() == value_constant::floating && rhs.type() == value_constant::integer)
        {
            return lhs.data().floating == static_cast<float_type>(rhs.data().integer);
        }
        return false;
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
            case value_constant::array:
                return (*lhs.data().vector) < (*rhs.data().vector);

            case value_constant::object:
                return (*lhs.data().object) < (*rhs.data().object);

            case value_constant::null:
                return false;

            case value_constant::string:
                return (*lhs.data().string) < (*rhs.data().string);

            case value_constant::boolean:
                return (lhs.data().boolean < rhs.data().boolean);

            case value_constant::integer:
                return (lhs.data().integer < rhs.data().integer);

            case value_constant::floating:
                return (lhs.data().floating < rhs.data().floating);

            default:
                return false;
            }
        }
        else if (lhs_type == value_constant::integer && rhs_type == value_constant::floating)
        {
            return (static_cast<float_type>(lhs.data().integer) < rhs.data().floating);
        }
        else if (lhs_type == value_constant::floating && rhs_type == value_constant::integer)
        {
            return (lhs.data().floating < static_cast<float_type>(rhs.data().integer));
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

private:
    value_constant::type type_;
    data_type            data_;
};

namespace detail
{
template <typename _ValTy>
class value_constructor
{
public:
    using value_type = _ValTy;

    template <typename _Ty>
    using allocator_type = typename value_type::template allocator_type<_Ty>;

    template <value_constant::type _Ty>
    using type_constant = std::integral_constant<value_constant::type, _Ty>;

    template <value_constant::type _Ty, typename... _Args>
    static void reset(value_type& v, _Args&&... args)
    {
        destroy(v);
        v.type(_Ty);
        construct<_Ty>(v, std::forward<_Args>(args)...);
    }

    template <value_constant::type _Ty, typename... _Args>
    static inline void construct(value_type& v, _Args&&... args)
    {
        construct(type_constant<_Ty>{}, v, std::forward<_Args>(args)...);
    }

    static void destroy(value_type& v)
    {
        switch (v.type())
        {
        case value_constant::object:
            destroy<typename value_type::object_type>(v.data().object);
            break;
        case value_constant::array:
            destroy<typename value_type::array_type>(v.data().vector);
            break;
        case value_constant::string:
            destroy<typename value_type::string_type>(v.data().string);
            break;
        default:
            break;
        }
    }

private:
    template <typename... _Args>
    static inline void construct(type_constant<value_constant::object>, value_type& v, _Args&&... args)
    {
        v.data().object = create_data<typename value_type::object_type>(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    static inline void construct(type_constant<value_constant::array>, value_type& v, _Args&&... args)
    {
        v.data().vector = create_data<typename value_type::array_type>(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    static inline void construct(type_constant<value_constant::string>, value_type& v, _Args&&... args)
    {
        v.data().string = create_data<typename value_type::string_type>(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    static inline void construct(type_constant<value_constant::integer>, value_type& v, _Args&&... args)
    {
        v.data().integer = typename value_type::integer_type{ std::forward<_Args>(args)... };
    }

    template <typename... _Args>
    static inline void construct(type_constant<value_constant::floating>, value_type& v, _Args&&... args)
    {
        v.data().floating = typename value_type::float_type{ std::forward<_Args>(args)... };
    }

    template <typename... _Args>
    static inline void construct(type_constant<value_constant::boolean>, value_type& v, _Args&&... args)
    {
        v.data().boolean = typename value_type::boolean_type{ std::forward<_Args>(args)... };
    }

    template <typename... _Args>
    static inline void construct(type_constant<value_constant::null>, value_type& v, _Args&&... args)
    {
        ((void)v);
    }

    template <typename _Ty, typename... _Args>
    static _Ty* create_data(_Args&&... args)
    {
        using allocator_traits = std::allocator_traits<allocator_type<_Ty>>;

        static allocator_type<_Ty> allocator;

        auto ptr = allocator_traits::allocate(allocator, 1);
        allocator_traits::construct(allocator, ptr, std::forward<_Args>(args)...);
        return ptr;
    }

    template <typename _Ty>
    static void destroy(_Ty* ptr)
    {
        using allocator_traits = std::allocator_traits<allocator_type<_Ty>>;

        static allocator_type<_Ty> allocator;
        allocator_traits::destroy(allocator, ptr);
        allocator_traits::deallocate(allocator, ptr, 1);
    }
};

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

        operator value_type() const
        {
            value_type v{ value_constant::object };
            std::for_each(list_.begin(), list_.end(),
                          [&](const pair_type& pair) { v.data().object->emplace(pair.first, pair.second); });
            return v;
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

        operator value_type() const
        {
            value_type v{ value_constant::array };
            v.data().vector->reserve(list_.size());
            v.data().vector->assign(list_.begin(), list_.end());
            return v;
        }

    private:
        std::initializer_list<value_type> list_;
    };
};
}  // namespace detail

template <typename _ValTy>
inline _ValTy make_object(std::initializer_list<std::pair<typename _ValTy::string_type, _ValTy>> list)
{
    return typename detail::value_maker<_ValTy>::object{ list };
}

template <typename _ValTy>
inline _ValTy make_array(std::initializer_list<_ValTy> list)
{
    return typename detail::value_maker<_ValTy>::array{ list };
}

}  // namespace configor
