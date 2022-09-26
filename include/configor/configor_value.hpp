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
class value_accessor;

template <typename _ValTy>
class value_maker;
}  // namespace detail

template <typename _Args>
class basic_value final : public value_base
{
    friend detail::iterator<basic_value>;
    friend detail::iterator<const basic_value>;
    friend detail::value_accessor<basic_value>;
    friend detail::value_maker<basic_value>;

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

public:
    basic_value(std::nullptr_t = nullptr)
        : value_base{ value_base::null }
        , data_{}
    {
    }

    basic_value(const value_base::value_type t)
        : value_base{ t }
        , data_{}
    {
        using accessor = detail::value_accessor<basic_value>;
        switch (t)
        {
        case value_base::object:
            accessor::template construct_data<value_base::object>(*this);
            break;
        case value_base::array:
            accessor::template construct_data<value_base::array>(*this);
            break;
        case value_base::string:
            accessor::template construct_data<value_base::string>(*this);
            break;
        case value_base::integer:
            accessor::template construct_data<value_base::integer>(*this, integer_type(0));
            break;
        case value_base::floating:
            accessor::template construct_data<value_base::floating>(*this, float_type(0.0));
            break;
        case value_base::boolean:
            accessor::template construct_data<value_base::boolean>(*this, boolean_type(false));
            break;
        default:
            break;
        }
    }

    basic_value(const basic_value& other)
        : value_base{ other.type() }
        , data_{}
    {
        using accessor = detail::value_accessor<basic_value>;
        switch (other.type())
        {
        case value_base::object:
            accessor::template construct_data<value_base::object>(*this, *other.data_.object);
            break;
        case value_base::array:
            accessor::template construct_data<value_base::array>(*this, *other.data_.vector);
            break;
        case value_base::string:
            accessor::template construct_data<value_base::string>(*this, *other.data_.string);
            break;
        case value_base::integer:
            accessor::template construct_data<value_base::integer>(*this, other.data_.integer);
            break;
        case value_base::floating:
            accessor::template construct_data<value_base::floating>(*this, other.data_.floating);
            break;
        case value_base::boolean:
            accessor::template construct_data<value_base::boolean>(*this, other.data_.boolean);
            break;
        default:
            break;
        }
    }

    basic_value(basic_value&& other) noexcept
        : value_base{ other.type() }
        , data_{ other.data_ }
    {
        other.set_type(value_base::null);
        other.data_.object = nullptr;
    }

    template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
              typename std::enable_if<!is_value<_UTy>::value && detail::has_to_value<basic_value, _UTy>::value,
                                      int>::type = 0>
    basic_value(_CompatibleTy&& value)
    {
        binder_type<_UTy>::to_value(*this, std::forward<_CompatibleTy>(value));
    }

    ~basic_value()
    {
        using accessor = detail::value_accessor<basic_value>;
        accessor::destroy_data(*this);
    }

    inline bool is_object() const
    {
        return type() == value_base::object;
    }

    inline bool is_array() const
    {
        return type() == value_base::array;
    }

    inline bool is_string() const
    {
        return type() == value_base::string;
    }

    inline bool is_bool() const
    {
        return type() == value_base::boolean;
    }

    inline bool is_integer() const
    {
        return type() == value_base::integer;
    }

    inline bool is_float() const
    {
        return type() == value_base::floating;
    }

    inline bool is_number() const
    {
        return is_integer() || is_float();
    }

    inline bool is_null() const
    {
        return type() == value_base::null;
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
        case value_base::null:
            return 0;
        case value_base::array:
            return data_.vector->size();
        case value_base::object:
            return data_.object->size();
        default:
            return 1;
        }
    }

    bool empty() const
    {
        if (is_null())
            return true;

        if (is_object())
            return data_.object->empty();

        if (is_array())
            return data_.vector->empty();

        return false;
    }

    iterator find(const typename object_type::key_type& key)
    {
        if (is_object())
        {
            iterator iter(this);
            iter.object_it_ = data_.object->find(key);
            return iter;
        }
        return end();
    }

    const_iterator find(const typename object_type::key_type& key) const
    {
        if (is_object())
        {
            const_iterator iter(this);
            iter.object_it_ = data_.object->find(key);
            return iter;
        }
        return cend();
    }

    inline size_type count(const typename object_type::key_type& key) const
    {
        return is_object() ? data_.object->count(key) : 0;
    }

    size_type erase(const typename object_type::key_type& key)
    {
        if (!is_object())
        {
            throw configor_invalid_key("cannot use erase() with non-object value");
        }
        return data_.object->erase(key);
    }

    void erase(const size_type index)
    {
        if (!is_array())
        {
            throw configor_invalid_key("cannot use erase() with non-array value");
        }
        data_.vector->erase(data_.vector->begin() + static_cast<difference_type>(index));
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy pos)
    {
        _IterTy result = end();

        switch (type())
        {
        case value_base::object:
        {
            result.object_it_ = data_.object->erase(pos.object_it_);
            break;
        }

        case value_base::array:
        {
            result.array_it_ = data_.vector->erase(pos.array_it_);
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
        case value_base::object:
        {
            result.object_it_ = data_.object->erase(first.object_it_, last.object_it_);
            break;
        }

        case value_base::array:
        {
            result.array_it_ = data_.vector->erase(first.array_it_, last.array_it_);
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
            *this = value_base::array;
        }
        data_.vector->emplace_back(std::forward<_Ty>(v)...);
    }

    void clear()
    {
        switch (type())
        {
        case value_base::integer:
            data_.integer = 0;
            break;
        case value_base::floating:
            data_.floating = static_cast<float_type>(0.0);
            break;
        case value_base::boolean:
            data_.boolean = false;
            break;
        case value_base::string:
            data_.string->clear();
            break;
        case value_base::array:
            data_.vector->clear();
            break;
        case value_base::object:
            data_.object->clear();
            break;
        default:
            break;
        }
    }

private:
    // get pointer value

    inline boolean_type* do_get_ptr(boolean_type*) noexcept
    {
        return is_bool() ? &data_.boolean : nullptr;
    }

    inline const boolean_type* do_get_ptr(const boolean_type*) const noexcept
    {
        return is_bool() ? &data_.boolean : nullptr;
    }

    inline integer_type* do_get_ptr(integer_type*) noexcept
    {
        return is_integer() ? &data_.integer : nullptr;
    }

    inline const integer_type* do_get_ptr(const integer_type*) const noexcept
    {
        return is_integer() ? &data_.integer : nullptr;
    }

    inline float_type* do_get_ptr(float_type*) noexcept
    {
        return is_float() ? &data_.floating : nullptr;
    }

    inline const float_type* do_get_ptr(const float_type*) const noexcept
    {
        return is_float() ? &data_.floating : nullptr;
    }

    inline string_type* do_get_ptr(string_type*) noexcept
    {
        return is_string() ? data_.string : nullptr;
    }

    inline const string_type* do_get_ptr(const string_type*) const noexcept
    {
        return is_string() ? data_.string : nullptr;
    }

    inline array_type* do_get_ptr(array_type*) noexcept
    {
        return is_array() ? data_.vector : nullptr;
    }

    inline const array_type* do_get_ptr(const array_type*) const noexcept
    {
        return is_array() ? data_.vector : nullptr;
    }

    inline object_type* do_get_ptr(object_type*) noexcept
    {
        return is_object() ? data_.object : nullptr;
    }

    inline const object_type* do_get_ptr(const object_type*) const noexcept
    {
        return is_object() ? data_.object : nullptr;
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
            *this = value_base::array;
        }

        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array object");
        }

        if (index >= data_.vector->size())
        {
            data_.vector->insert(data_.vector->end(), index - data_.vector->size() + 1, basic_value());
        }
        return (*data_.vector)[index];
    }

    inline basic_value& operator[](const size_type index) const
    {
        return at(index);
    }

    basic_value& operator[](const typename object_type::key_type& key)
    {
        if (is_null())
        {
            *this = value_base::object;
        }
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object type");
        }
        return (*data_.object)[key];
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
        if (index >= data_.vector->size())
        {
            throw std::out_of_range("operator[] index out of range");
        }
        return (*data_.vector)[index];
    }

    template <typename _CharTy>
    basic_value& operator[](_CharTy* key)
    {
        if (is_null())
        {
            *this = value_base::object;
        }

        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }
        return (*data_.object)[key];
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

        auto iter = data_.object->find(key);
        if (iter == data_.object->end())
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

        auto iter = data_.object->find(key);
        if (iter == data_.object->end())
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
            case value_base::array:
                return (*lhs.data_.vector == *rhs.data_.vector);
            case value_base::object:
                return (*lhs.data_.object == *rhs.data_.object);
            case value_base::null:
                return true;
            case value_base::string:
                return *lhs.data_.string == *rhs.data_.string;
            case value_base::boolean:
                return lhs.data_.boolean == rhs.data_.boolean;
            case value_base::integer:
                return lhs.data_.integer == rhs.data_.integer;
            case value_base::floating:
                return lhs.data_.floating == rhs.data_.floating;
            default:
                return false;
            }
        }
        else if (lhs.type() == value_base::integer && rhs.type() == value_base::floating)
        {
            return static_cast<float_type>(lhs.data_.integer) == rhs.data_.floating;
        }
        else if (lhs.type() == value_base::floating && rhs.type() == value_base::integer)
        {
            return lhs.data_.floating == static_cast<float_type>(rhs.data_.integer);
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
            case value_base::array:
                return (*lhs.data_.vector) < (*rhs.data_.vector);

            case value_base::object:
                return (*lhs.data_.object) < (*rhs.data_.object);

            case value_base::null:
                return false;

            case value_base::string:
                return (*lhs.data_.string) < (*rhs.data_.string);

            case value_base::boolean:
                return (lhs.data_.boolean < rhs.data_.boolean);

            case value_base::integer:
                return (lhs.data_.integer < rhs.data_.integer);

            case value_base::floating:
                return (lhs.data_.floating < rhs.data_.floating);

            default:
                return false;
            }
        }
        else if (lhs_type == value_base::integer && rhs_type == value_base::floating)
        {
            return (static_cast<float_type>(lhs.data_.integer) < rhs.data_.floating);
        }
        else if (lhs_type == value_base::floating && rhs_type == value_base::integer)
        {
            return (lhs.data_.floating < static_cast<float_type>(rhs.data_.integer));
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
private:
    union data_type
    {
        boolean_type boolean;
        integer_type integer;
        float_type   floating;
        string_type* string;
        object_type* object;
        array_type*  vector;
    } data_;
};

namespace detail
{
template <typename _ValTy>
class value_accessor
{
public:
    using value_type = _ValTy;

    template <typename _Ty>
    using allocator_type = typename value_type::template allocator_type<_Ty>;

    static inline typename value_type::data_type& get_data(value_type& v)
    {
        return v.data_;
    }

    static inline const typename value_type::data_type& get_data(const value_type& v)
    {
        return v.data_;
    }

    template <value_base::value_type _Ty>
    using type_constant = std::integral_constant<value_base::value_type, _Ty>;

    template <value_base::value_type _Ty, typename... _Args>
    static void reset_data(value_type& v, _Args&&... args)
    {
        destroy_data(v);
        v.set_type(_Ty);
        construct_data<_Ty>(v, std::forward<_Args>(args)...);
    }

    template <value_base::value_type _Ty, typename... _Args>
    static inline void construct_data(value_type& v, _Args&&... args)
    {
        construct_data(type_constant<_Ty>{}, v, std::forward<_Args>(args)...);
    }

    static void destroy_data(value_type& v)
    {
        switch (v.type())
        {
        case value_base::object:
            destroy_data<typename value_type::object_type>(v.data_.object);
            break;
        case value_base::array:
            destroy_data<typename value_type::array_type>(v.data_.vector);
            break;
        case value_base::string:
            destroy_data<typename value_type::string_type>(v.data_.string);
            break;
        default:
            break;
        }
    }

private:
    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::object>, value_type& v, _Args&&... args)
    {
        v.data_.object = create_data<typename value_type::object_type>(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::array>, value_type& v, _Args&&... args)
    {
        v.data_.vector = create_data<typename value_type::array_type>(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::string>, value_type& v, _Args&&... args)
    {
        v.data_.string = create_data<typename value_type::string_type>(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::integer>, value_type& v, _Args&&... args)
    {
        v.data_.integer = typename value_type::integer_type{ std::forward<_Args>(args)... };
    }

    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::floating>, value_type& v, _Args&&... args)
    {
        v.data_.floating = typename value_type::float_type{ std::forward<_Args>(args)... };
    }

    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::boolean>, value_type& v, _Args&&... args)
    {
        v.data_.boolean = typename value_type::boolean_type{ std::forward<_Args>(args)... };
    }

    template <typename... _Args>
    static inline void construct_data(type_constant<value_base::null>, value_type& v, _Args&&... args)
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
    static void destroy_data(_Ty* ptr)
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
            value_type v{ value_base::object };
            std::for_each(list_.begin(), list_.end(),
                          [&](const pair_type& pair) { v.data_.object->emplace(pair.first, pair.second); });
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
            value_type v{ value_base::array };
            v.data_.vector->reserve(list_.size());
            v.data_.vector->assign(list_.begin(), list_.end());
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
