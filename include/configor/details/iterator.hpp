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
#include "declare.hpp"
#include "exception.hpp"

#include <cstddef>   // std::ptrdiff_t
#include <iterator>  // std::reverse_iterator, std::prev
#include <memory>    // std::addressof

namespace configor
{
namespace detail
{

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

template <typename _ValTy>
struct iterator
{
    friend _ValTy;
    friend iterator<typename std::conditional<std::is_const<_ValTy>::value, typename std::remove_const<_ValTy>::type,
                                              const _ValTy>::type>;

    using value_type        = _ValTy;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer           = value_type*;
    using reference         = value_type&;

    inline explicit iterator(value_type* v)
        : value_(v)
    {
    }

    inline iterator(const iterator<const _ValTy>& rhs)
        : value_(rhs.value_)
        , array_it_(rhs.array_it_)
        , object_it_(rhs.object_it_)
        , primitive_it_(rhs.primitive_it_)
    {
    }

    iterator& operator=(const iterator<const _ValTy>& rhs)
    {
        this->value_        = rhs.value_;
        this->array_it_     = rhs.array_it_;
        this->object_it_    = rhs.object_it_;
        this->primitive_it_ = rhs.primitive_it_;
        return *this;
    }

    inline iterator(const iterator<typename std::remove_const<_ValTy>::type>& rhs)
        : value_(rhs.value_)
        , array_it_(rhs.array_it_)
        , object_it_(rhs.object_it_)
        , primitive_it_(rhs.primitive_it_)
    {
    }

    iterator& operator=(const iterator<typename std::remove_const<_ValTy>::type>& rhs)
    {
        this->value_        = rhs.value_;
        this->array_it_     = rhs.array_it_;
        this->object_it_    = rhs.object_it_;
        this->primitive_it_ = rhs.primitive_it_;
        return *this;
    }

    const typename value_type::string_type& key() const
    {
        check_data();
        check_iterator();

        if (value_->type() != value_constant::object)
            throw configor_invalid_iterator("cannot use key() with non-object type");
        return object_it_->first;
    }

    reference value() const
    {
        check_data();
        check_iterator();

        switch (value_->type())
        {
        case value_constant::object:
            return object_it_->second;
        case value_constant::array:
            return *array_it_;
        default:
            break;
        }
        return *value_;
    }

    inline reference operator*() const
    {
        return this->value();
    }

    inline pointer operator->() const
    {
        return std::addressof(this->operator*());
    }

    inline iterator operator++(int)
    {
        iterator old = (*this);
        ++(*this);
        return old;
    }

    iterator& operator++()
    {
        check_data();

        switch (value_->type())
        {
        case value_constant::object:
        {
            std::advance(object_it_, 1);
            break;
        }
        case value_constant::array:
        {
            std::advance(array_it_, 1);
            break;
        }
        case value_constant::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            ++primitive_it_;
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

    iterator& operator--()
    {
        check_data();

        switch (value_->type())
        {
        case value_constant::object:
        {
            std::advance(object_it_, -1);
            break;
        }
        case value_constant::array:
        {
            std::advance(array_it_, -1);
            break;
        }
        case value_constant::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            --primitive_it_;
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
    iterator& operator+=(difference_type off)
    {
        check_data();

        switch (value_->type())
        {
        case value_constant::object:
        {
            throw configor_invalid_iterator("cannot compute offsets with object type");
            break;
        }
        case value_constant::array:
        {
            std::advance(array_it_, off);
            break;
        }
        case value_constant::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            primitive_it_ += off;
            break;
        }
        }
        return *this;
    }

    difference_type operator-(const iterator& rhs) const
    {
        check_data();
        rhs.check_data();

        if (value_ != rhs.value_)
            throw configor_invalid_iterator("cannot compute iterator offsets of different value objects");

        if (value_->type() != value_constant::array)
            throw configor_invalid_iterator("cannot compute iterator offsets with non-array type");
        return array_it_ - rhs.array_it_;
    }

    inline bool operator!=(iterator const& other) const
    {
        return !(*this == other);
    }
    bool operator==(iterator const& other) const
    {
        if (value_ == nullptr)
            return false;

        if (value_ != other.value_)
            return false;

        switch (value_->type())
        {
        case value_constant::object:
        {
            return object_it_ == other.object_it_;
        }
        case value_constant::array:
        {
            return array_it_ == other.array_it_;
        }
        default:
        {
            return primitive_it_ == other.primitive_it_;
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
    bool operator<(iterator const& other) const
    {
        check_data();
        other.check_data();

        if (value_ != other.value_)
            throw configor_invalid_iterator("cannot compare iterators of different value objects");

        switch (value_->type())
        {
        case value_constant::object:
            throw configor_invalid_iterator("cannot compare iterators with object type");
        case value_constant::array:
            return array_it_ < other.array_it_;
        default:
            return primitive_it_ < other.primitive_it_;
        }
    }

private:
    void set_begin()
    {
        check_data();

        switch (value_->type())
        {
        case value_constant::object:
        {
            object_it_ = value_->data().object->begin();
            break;
        }
        case value_constant::array:
        {
            array_it_ = value_->data().vector->begin();
            break;
        }
        case value_constant::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            primitive_it_.set_begin();
            break;
        }
        }
    }

    void set_end()
    {
        check_data();

        switch (value_->type())
        {
        case value_constant::object:
        {
            object_it_ = value_->data().object->end();
            break;
        }
        case value_constant::array:
        {
            array_it_ = value_->data().vector->end();
            break;
        }
        case value_constant::null:
        {
            // DO NOTHING
            break;
        }
        default:
        {
            primitive_it_.set_end();
            break;
        }
        }
    }

    inline void check_data() const
    {
        if (value_ == nullptr)
        {
            throw configor_invalid_iterator("iterator is empty");
        }
    }

    void check_iterator() const
    {
        switch (value_->type())
        {
        case value_constant::object:
            if (object_it_ == value_->data().object->end())
            {
                throw std::out_of_range("object iterator out of range");
            }
            break;
        case value_constant::array:
            if (array_it_ == value_->data().vector->end())
            {
                throw std::out_of_range("array iterator out of range");
            }
            break;
        case value_constant::null:
        {
            throw std::out_of_range("null iterator out of range");
        }
        default:
            if (primitive_it_ != 0)
            {
                throw std::out_of_range("primitive iterator out of range");
            }
            break;
        }
    }

private:
    value_type* value_;

    typename _ValTy::array_type::iterator  array_it_;
    typename _ValTy::object_type::iterator object_it_;
    primitive_iterator                     primitive_it_ = 0;  // for other types
};

template <typename _IterTy>
struct reverse_iterator : public std::reverse_iterator<_IterTy>
{
    using iterator_type     = typename std::reverse_iterator<_IterTy>::iterator_type;
    using value_type        = typename std::reverse_iterator<_IterTy>::value_type;
    using difference_type   = typename std::reverse_iterator<_IterTy>::difference_type;
    using iterator_category = typename std::reverse_iterator<_IterTy>::iterator_category;
    using pointer           = typename std::reverse_iterator<_IterTy>::pointer;
    using reference         = typename std::reverse_iterator<_IterTy>::reference;

    reverse_iterator() = default;

    inline explicit reverse_iterator(iterator_type it)
        : std::reverse_iterator<_IterTy>(it)
    {
    }

    inline const typename value_type::string_type& key() const
    {
        return std::prev(this->current).key();
    }

    inline reference value() const
    {
        return std::prev(this->current).value();
    }
};

}  // namespace detail
}  // namespace configor
