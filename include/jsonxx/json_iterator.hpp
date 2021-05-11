// Copyright (c) 2018-2020 jsonxx - Nomango
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
#include "json_exception.hpp"
#include "json_value.hpp"

#include <cstddef>  // std::ptrdiff_t

namespace jsonxx
{
namespace detail
{
//
// iterator for basic_json
//

struct primitive_iterator
{
    using difference_type = std::ptrdiff_t;

    inline primitive_iterator(difference_type it = 0)
        : it_(it)
    {
    }

    inline void set_begin()
    {
        it_ = 0;
    }
    inline void set_end()
    {
        it_ = 1;
    }

    inline primitive_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    inline primitive_iterator operator++(int)
    {
        primitive_iterator old(it_);
        ++(*this);
        return old;
    }

    inline primitive_iterator& operator--()
    {
        --it_;
        return (*this);
    }
    inline primitive_iterator operator--(int)
    {
        primitive_iterator old = (*this);
        --(*this);
        return old;
    }

    inline bool operator==(primitive_iterator const& other) const
    {
        return it_ == other.it_;
    }
    inline bool operator!=(primitive_iterator const& other) const
    {
        return !(*this == other);
    }

    inline const primitive_iterator operator+(difference_type off) const
    {
        return primitive_iterator(it_ + off);
    }
    inline const primitive_iterator operator-(difference_type off) const
    {
        return primitive_iterator(it_ - off);
    }

    inline primitive_iterator& operator+=(difference_type off)
    {
        it_ += off;
        return (*this);
    }
    inline primitive_iterator& operator-=(difference_type off)
    {
        it_ -= off;
        return (*this);
    }

    inline difference_type operator-(primitive_iterator const& other) const
    {
        return it_ - other.it_;
    }

    inline bool operator<(primitive_iterator const& other) const
    {
        return it_ < other.it_;
    }
    inline bool operator<=(primitive_iterator const& other) const
    {
        return it_ <= other.it_;
    }
    inline bool operator>(primitive_iterator const& other) const
    {
        return it_ > other.it_;
    }
    inline bool operator>=(primitive_iterator const& other) const
    {
        return it_ >= other.it_;
    }

private:
    difference_type it_;
};

template <typename _BasicJsonTy>
struct iterator_value
{
    using value_type  = _BasicJsonTy;
    using object_type = typename _BasicJsonTy::object_type;
    using key_type    = typename object_type::key_type;

    explicit iterator_value(value_type* value)
        : key_(&dummy_key_)
        , value_(value)
    {
    }

    explicit iterator_value(const key_type& key, value_type* value)
        : key_(&key)
        , value_(value)
    {
    }

    inline const key_type& key() const
    {
        if (key_ == &dummy_key_)
            throw json_invalid_iterator("cannot use key() with non-object type");
        return *key_;
    }

    inline value_type& value() const
    {
        return *value_;
    }

    inline operator value_type&() const
    {
        return *value_;
    }

private:
    static key_type dummy_key_;

    const key_type* key_;
    value_type*     value_;
};

template <typename _BasicJsonTy>
typename iterator_value<_BasicJsonTy>::key_type iterator_value<_BasicJsonTy>::dummy_key_;

template <typename _BasicJsonTy>
struct iterator
{
    friend _BasicJsonTy;

    using string_type  = typename _BasicJsonTy::string_type;
    using char_type    = typename _BasicJsonTy::char_type;
    using integer_type = typename _BasicJsonTy::integer_type;
    using float_type   = typename _BasicJsonTy::float_type;
    using boolean_type = typename _BasicJsonTy::boolean_type;
    using array_type   = typename _BasicJsonTy::array_type;
    using object_type  = typename _BasicJsonTy::object_type;

    using value_type        = _BasicJsonTy;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer           = iterator_value<value_type>*;
    using reference         = iterator_value<value_type>&;

    inline explicit iterator(value_type* json)
        : data_(json)
        , it_value_(json)
    {
    }

    inline reference operator*() const
    {
        check_data();
        check_iterator();

        switch (data_->type())
        {
        case json_type::object:
            it_value_ = iterator_value<value_type>(object_it_->first, &(object_it_->second));
            break;
        case json_type::array:
            it_value_ = iterator_value<value_type>(&(*array_it_));
            break;
        default:
            it_value_ = iterator_value<value_type>(data_);
            break;
        }
        return it_value_;
    }

    inline pointer operator->() const
    {
        return &(operator*());
    }

    inline iterator operator++(int)
    {
        iterator old = (*this);
        ++(*this);
        return old;
    }

    inline iterator& operator++()
    {
        check_data();

        switch (data_->type())
        {
        case json_type::object:
        {
            std::advance(object_it_, 1);
            break;
        }
        case json_type::array:
        {
            std::advance(array_it_, 1);
            break;
        }
        case json_type::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            ++original_it_;
            break;
        }
        }
        return *this;
    }

    inline iterator operator--(int)
    {
        iterator old = (*this);
        --(*this);
        return old;
    }

    inline iterator& operator--()
    {
        check_data();

        switch (data_->type())
        {
        case json_type::object:
        {
            std::advance(object_it_, -1);
            break;
        }
        case json_type::array:
        {
            std::advance(array_it_, -1);
            break;
        }
        case json_type::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            --original_it_;
            break;
        }
        }
        return *this;
    }

    inline const iterator operator-(difference_type off) const
    {
        return operator+(-off);
    }
    inline const iterator operator+(difference_type off) const
    {
        iterator ret(*this);
        ret += off;
        return ret;
    }

    inline iterator& operator-=(difference_type off)
    {
        return operator+=(-off);
    }
    inline iterator& operator+=(difference_type off)
    {
        check_data();

        switch (data_->type())
        {
        case json_type::object:
        {
            throw json_invalid_iterator("cannot use offsets with object type");
            break;
        }
        case json_type::array:
        {
            std::advance(array_it_, off);
            break;
        }
        case json_type::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            original_it_ += off;
            break;
        }
        }
        return *this;
    }

    inline bool operator!=(iterator const& other) const
    {
        return !(*this == other);
    }
    inline bool operator==(iterator const& other) const
    {
        if (data_ == nullptr)
            return false;

        if (data_ != other.data_)
            return false;

        switch (data_->type())
        {
        case json_type::object:
        {
            return object_it_ == other.object_it_;
        }
        case json_type::array:
        {
            return array_it_ == other.array_it_;
        }
        default:
        {
            return original_it_ == other.original_it_;
        }
        }
    }

    inline bool operator>(iterator const& other) const
    {
        return other.operator<(*this);
    }
    inline bool operator>=(iterator const& other) const
    {
        return !operator<(other);
    }
    inline bool operator<=(iterator const& other) const
    {
        return !other.operator<(*this);
    }
    inline bool operator<(iterator const& other) const
    {
        check_data();
        other.check_data();

        if (data_ != other.data_)
            throw json_invalid_iterator("cannot compare iterators of different objects");

        switch (data_->type())
        {
        case json_type::object:
            throw json_invalid_iterator("cannot compare iterators with object type");
        case json_type::array:
            return array_it_ < other.array_it_;
        default:
            return original_it_ < other.original_it_;
        }
    }

private:
    inline void set_begin()
    {
        check_data();

        switch (data_->type())
        {
        case json_type::object:
        {
            object_it_ = data_->value_.data.object->begin();
            break;
        }
        case json_type::array:
        {
            array_it_ = data_->value_.data.vector->begin();
            break;
        }
        case json_type::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            original_it_.set_begin();
            break;
        }
        }
    }

    inline void set_end()
    {
        check_data();

        switch (data_->type())
        {
        case json_type::object:
        {
            object_it_ = data_->value_.data.object->end();
            break;
        }
        case json_type::array:
        {
            array_it_ = data_->value_.data.vector->end();
            break;
        }
        case json_type::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            original_it_.set_end();
            break;
        }
        }
    }

    inline void check_data() const
    {
        if (data_ == nullptr)
        {
            throw json_invalid_iterator("iterator contains an empty object");
        }
    }

    inline void check_iterator() const
    {
        switch (data_->type())
        {
        case json_type::object:
            if (object_it_ == data_->value_.data.object->end())
            {
                throw std::out_of_range("iterator out of range");
            }
            break;
        case json_type::array:
            if (array_it_ == data_->value_.data.vector->end())
            {
                throw std::out_of_range("iterator out of range");
            }
            break;
        case json_type::null:
        {
            throw std::out_of_range("iterator out of range");
        }
        default:
            if (original_it_ != 0)
            {
                throw std::out_of_range("iterator out of range");
            }
            break;
        }
    }

private:
    value_type* data_;

    mutable iterator_value<value_type> it_value_;

    typename _BasicJsonTy::array_type::iterator  array_it_;
    typename _BasicJsonTy::object_type::iterator object_it_;
    primitive_iterator                           original_it_ = 0;  // for other types
};

}  // namespace detail
}  // namespace jsonxx
