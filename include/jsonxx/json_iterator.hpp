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
#include <cstddef>

namespace jsonxx
{
    //
    // iterator for basic_json
    //

    struct primitive_iterator
    {
        using difference_type = std::ptrdiff_t;

        inline primitive_iterator(difference_type it = 0) : it_(it) {}

        inline void set_begin() { it_ = 0; }
        inline void set_end() { it_ = 1; }

        inline primitive_iterator &operator++()
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

        inline primitive_iterator &operator--()
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

        inline bool operator==(primitive_iterator const &other) const { return it_ == other.it_; }
        inline bool operator!=(primitive_iterator const &other) const { return !(*this == other); }

        inline const primitive_iterator operator+(difference_type off) const { return primitive_iterator(it_ + off); }
        inline const primitive_iterator operator-(difference_type off) const { return primitive_iterator(it_ - off); }

        inline primitive_iterator &operator+=(difference_type off)
        {
            it_ += off;
            return (*this);
        }
        inline primitive_iterator &operator-=(difference_type off)
        {
            it_ -= off;
            return (*this);
        }

        inline difference_type operator-(primitive_iterator const &other) const { return it_ - other.it_; }

        inline bool operator<(primitive_iterator const &other) const { return it_ < other.it_; }
        inline bool operator<=(primitive_iterator const &other) const { return it_ <= other.it_; }
        inline bool operator>(primitive_iterator const &other) const { return it_ > other.it_; }
        inline bool operator>=(primitive_iterator const &other) const { return it_ >= other.it_; }

    private:
        difference_type it_;
    };

    template <typename _BasicJsonTy>
    struct internal_iterator
    {
        typename _BasicJsonTy::array_type::iterator array_iter;
        typename _BasicJsonTy::object_type::iterator object_iter;
        primitive_iterator original_iter = 0; // for other types
    };

    template <typename _BasicJsonTy>
    struct iterator_impl
    {
        friend _BasicJsonTy;

        using string_type = typename _BasicJsonTy::string_type;
        using char_type = typename _BasicJsonTy::char_type;
        using integer_type = typename _BasicJsonTy::integer_type;
        using float_type = typename _BasicJsonTy::float_type;
        using boolean_type = typename _BasicJsonTy::boolean_type;
        using array_type = typename _BasicJsonTy::array_type;
        using object_type = typename _BasicJsonTy::object_type;

        using value_type = _BasicJsonTy;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer = value_type *;
        using reference = value_type &;

        inline iterator_impl(pointer json) : data_(json) {}

        inline reference operator*() const
        {
            check_data();
            check_iterator();
            switch (data_->type())
            {
            case json_type::object:
                return (it_.object_iter->second);
            case json_type::array:
                return (*it_.array_iter);
            default:
                return *data_;
            }
        }

        inline pointer operator->() const
        {
            check_data();
            check_iterator();
            switch (data_->type())
            {
            case json_type::object:
                return &(it_.object_iter->second);
            case json_type::array:
                return &(*it_.array_iter);
            default:
                return data_;
            }
        }

        inline const typename object_type::key_type &key() const
        {
            check_data();
            check_iterator();
            if (!data_->is_object())
                throw json_invalid_iterator("cannot use key() with non-object type");
            return it_.object_iter->first;
        }

        inline reference value() const
        {
            return operator*();
        }

        inline void set_begin()
        {
            check_data();

            switch (data_->type())
            {
            case json_type::object:
            {
                it_.object_iter = data_->value_.data.object->begin();
                break;
            }
            case json_type::array:
            {
                it_.array_iter = data_->value_.data.vector->begin();
                break;
            }
            default:
            {
                it_.original_iter.set_begin();
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
                it_.object_iter = data_->value_.data.object->end();
                break;
            }
            case json_type::array:
            {
                it_.array_iter = data_->value_.data.vector->end();
                break;
            }
            default:
            {
                it_.original_iter.set_end();
                break;
            }
            }
        }

        inline iterator_impl operator++(int)
        {
            iterator_impl old = (*this);
            ++(*this);
            return old;
        }
        inline iterator_impl &operator++()
        {
            check_data();

            switch (data_->type())
            {
            case json_type::object:
            {
                std::advance(it_.object_iter, 1);
                break;
            }
            case json_type::array:
            {
                std::advance(it_.array_iter, 1);
                break;
            }
            default:
            {
                ++it_.original_iter;
                break;
            }
            }
            return *this;
        }

        inline iterator_impl operator--(int)
        {
            iterator_impl old = (*this);
            --(*this);
            return old;
        }
        inline iterator_impl &operator--()
        {
            check_data();

            switch (data_->type())
            {
            case json_type::object:
            {
                std::advance(it_.object_iter, -1);
                break;
            }
            case json_type::array:
            {
                std::advance(it_.array_iter, -1);
                break;
            }
            default:
            {
                --it_.original_iter;
                break;
            }
            }
        }

        inline const iterator_impl operator-(difference_type off) const { return operator+(-off); }
        inline const iterator_impl operator+(difference_type off) const
        {
            iterator_impl ret(*this);
            ret += off;
            return ret;
        }

        inline iterator_impl &operator-=(difference_type off) { return operator+=(-off); }
        inline iterator_impl &operator+=(difference_type off)
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
                std::advance(it_.array_iter, off);
                break;
            }
            default:
            {
                it_.original_iter += off;
                break;
            }
            }
            return *this;
        }

        inline bool operator!=(iterator_impl const &other) const { return !(*this == other); }
        inline bool operator==(iterator_impl const &other) const
        {
            if (data_ != other.data_)
                return false;

            if (data_ == nullptr)
                throw json_invalid_iterator("json data is nullptr");

            switch (data_->type())
            {
            case json_type::object:
            {
                return it_.object_iter == other.it_.object_iter;
            }
            case json_type::array:
            {
                return it_.array_iter == other.it_.array_iter;
            }
            default:
            {
                return it_.original_iter == other.it_.original_iter;
            }
            }
        }

        inline bool operator>(iterator_impl const &other) const { return other.operator<(*this); }
        inline bool operator>=(iterator_impl const &other) const { return !operator<(other); }
        inline bool operator<=(iterator_impl const &other) const { return !other.operator<(*this); }
        inline bool operator<(iterator_impl const &other) const
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
                return it_.array_iter < other.it_.array_iter;
            default:
                return it_.original_iter < other.it_.original_iter;
            }
        }

    private:
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
                if (it_.object_iter == data_->value_.data.object->end())
                {
                    throw std::out_of_range("iterator out of range");
                }
                break;
            case json_type::array:
                if (it_.array_iter == data_->value_.data.vector->end())
                {
                    throw std::out_of_range("iterator out of range");
                }
                break;
            default:
                if (it_.original_iter == 1)
                {
                    throw std::out_of_range("iterator out of range");
                }
                break;
            }
        }

    private:
        pointer data_;
        internal_iterator<_BasicJsonTy> it_;
    };
} // namespace jsonxx
