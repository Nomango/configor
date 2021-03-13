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
#include <cmath>  // std::fabs
#include <limits>  // std::numeric_limits
#include <memory>  // std::allocator_traits

namespace jsonxx
{
    namespace detail
    {
        template <typename _Ty>
        bool nearly_equal(_Ty a, _Ty b)
        {
            return std::fabs(a - b) < std::numeric_limits<_Ty>::epsilon();
        }
    }

    //
    // json value type
    //

    enum class json_type
    {
        number_integer,
        number_float,
        string,
        array,
        object,
        boolean,
        null,
    };

    //
    // json_value
    //

    template <typename _BasicJsonTy>
    struct json_value
    {
        using string_type = typename _BasicJsonTy::string_type;
        using char_type = typename _BasicJsonTy::char_type;
        using integer_type = typename _BasicJsonTy::integer_type;
        using float_type = typename _BasicJsonTy::float_type;
        using boolean_type = typename _BasicJsonTy::boolean_type;
        using array_type = typename _BasicJsonTy::array_type;
        using object_type = typename _BasicJsonTy::object_type;

        json_type type;
        union
        {
            object_type *object;
            array_type *vector;
            string_type *string;
            integer_type number_integer;
            float_type number_float;
            boolean_type boolean;
        } data;

        json_value()
        {
            type = json_type::null;
            data.object = nullptr;
        }

        json_value(std::nullptr_t)
        {
            type = json_type::null;
            data.object = nullptr;
        }

        json_value(const object_type &value)
        {
            type = json_type::object;
            data.object = create<object_type>(value);
        }

        json_value(const array_type &value)
        {
            type = json_type::array;
            data.vector = create<array_type>(value);
        }

        json_value(const string_type &value)
        {
            type = json_type::string;
            data.string = create<string_type>(value);
        }

        template <typename _CharT>
        json_value(const _CharT *str)
        {
            type = json_type::string;
            data.string = create<string_type>(str);
        }

        json_value(const integer_type value)
        {
            type = json_type::number_integer;
            data.number_integer = value;
        }

        json_value(const float_type value)
        {
            type = json_type::number_float;
            data.number_float = value;
        }

        json_value(const boolean_type value)
        {
            type = json_type::boolean;
            data.boolean = value;
        }

        json_value(const json_type value_type)
        {
            type = value_type;
            switch (type)
            {
            case json_type::object:
                data.object = create<object_type>();
                break;
            case json_type::array:
                data.vector = create<array_type>();
                break;
            case json_type::string:
                data.string = create<string_type>();
                break;
            case json_type::number_integer:
                data.number_integer = integer_type(0);
                break;
            case json_type::number_float:
                data.number_float = float_type(0.0);
                break;
            case json_type::boolean:
                data.boolean = boolean_type(false);
                break;
            default:
                data.object = nullptr;
                break;
            }
        }

        json_value(json_value const &other)
        {
            type = other.type;

            switch (other.type)
            {
            case json_type::object:
                data.object = create<object_type>(*other.data.object);
                break;
            case json_type::array:
                data.vector = create<array_type>(*other.data.vector);
                break;
            case json_type::string:
                data.string = create<string_type>(*other.data.string);
                break;
            case json_type::number_integer:
                data.number_integer = other.data.number_integer;
                break;
            case json_type::number_float:
                data.number_float = other.data.number_float;
                break;
            case json_type::boolean:
                data.boolean = other.data.boolean;
                break;
            default:
                data.object = nullptr;
                break;
            }
        }

        json_value(json_value &&other)
        {
            type = other.type;
            data = other.data;
            other.type = json_type::null;
            other.data.object = nullptr;
        }

        ~json_value()
        {
            clear();
        }

        void swap(json_value &other)
        {
            std::swap(type, other.type);
            std::swap(data, other.data);
        }

        void clear()
        {
            switch (type)
            {
            case json_type::object:
                destroy<object_type>(data.object);
                break;
            case json_type::array:
                destroy<array_type>(data.vector);
                break;
            case json_type::string:
                destroy<string_type>(data.string);
                break;
            default:
                break;
            }
        }

        template <typename _Ty, typename... _Args>
        inline _Ty *create(_Args &&... args)
        {
            using allocator_type = typename _BasicJsonTy::template allocator_type<_Ty>;
            using allocator_traits = std::allocator_traits<allocator_type>;

            allocator_type allocator;
            _Ty *ptr = allocator_traits::allocate(allocator, 1);
            allocator_traits::construct(allocator, ptr, std::forward<_Args>(args)...);
            return ptr;
        }

        template <typename _Ty>
        inline void destroy(_Ty *ptr)
        {
            using allocator_type = typename _BasicJsonTy::template allocator_type<_Ty>;
            using allocator_traits = std::allocator_traits<allocator_type>;

            allocator_type allocator;
            allocator_traits::destroy(allocator, ptr);
            allocator_traits::deallocate(allocator, ptr, 1);
        }

        inline json_value &operator=(json_value const &other)
        {
            json_value{other}.swap(*this);
            return (*this);
        }

        inline json_value &operator=(json_value &&other)
        {
            clear();
            type = other.type;
            data = std::move(other.data);
            // invalidate payload
            other.type = json_type::null;
            other.data.object = nullptr;
            return (*this);
        }

        friend bool operator==(const json_value &lhs, const json_value &rhs)
        {
            if (lhs.type == rhs.type)
            {
                switch (lhs.type)
                {
                case json_type::array:
                    return (*lhs.data.vector == *rhs.data.vector);

                case json_type::object:
                    return (*lhs.data.object == *rhs.data.object);

                case json_type::null:
                    return true;

                case json_type::string:
                    return (*lhs.data.string == *rhs.data.string);

                case json_type::boolean:
                    return (lhs.data.boolean == rhs.data.boolean);

                case json_type::number_integer:
                    return (lhs.data.number_integer == rhs.data.number_integer);

                case json_type::number_float:
                    return detail::nearly_equal(lhs.data.number_float, rhs.data.number_float);

                default:
                    return false;
                }
            }
            else if (lhs.type == json_type::number_integer && rhs.type == json_type::number_float)
            {
                return detail::nearly_equal<float_type>(static_cast<float_type>(lhs.data.number_integer), rhs.data.number_float);
            }
            else if (lhs.type == json_type::number_float && rhs.type == json_type::number_integer)
            {
                return detail::nearly_equal<float_type>(lhs.data.number_float, static_cast<float_type>(rhs.data.number_integer));
            }
            return false;
        }
    };

    //
    // json_value_getter
    //

    template <typename _BasicJsonTy>
    struct json_value_getter
    {
        using string_type = typename _BasicJsonTy::string_type;
        using char_type = typename _BasicJsonTy::char_type;
        using integer_type = typename _BasicJsonTy::integer_type;
        using float_type = typename _BasicJsonTy::float_type;
        using boolean_type = typename _BasicJsonTy::boolean_type;
        using array_type = typename _BasicJsonTy::array_type;
        using object_type = typename _BasicJsonTy::object_type;

        static inline void assign(const _BasicJsonTy &json, object_type &value)
        {
            if (!json.is_object())
                throw json_type_error("json value type must be object");
            value = *json.value_.data.object;
        }

        static inline void assign(const _BasicJsonTy &json, array_type &value)
        {
            if (!json.is_array())
                throw json_type_error("json value type must be array");
            value = *json.value_.data.vector;
        }

        static inline void assign(const _BasicJsonTy &json, string_type &value)
        {
            if (!json.is_string())
                throw json_type_error("json value type must be string");
            value = *json.value_.data.string;
        }

        static inline void assign(const _BasicJsonTy &json, boolean_type &value)
        {
            if (!json.is_bool())
                throw json_type_error("json value type must be boolean");
            value = json.value_.data.boolean;
        }

        static inline void assign(const _BasicJsonTy &json, integer_type &value)
        {
            if (!json.is_integer())
                throw json_type_error("json value type must be integer");
            value = json.value_.data.number_integer;
        }

        template <
            typename _IntegerTy,
            typename std::enable_if<std::is_integral<_IntegerTy>::value, int>::type = 0>
        static inline void assign(const _BasicJsonTy &json, _IntegerTy &value)
        {
            if (!json.is_integer())
                throw json_type_error("json value type must be integer");
            value = static_cast<_IntegerTy>(json.value_.data.number_integer);
        }

        static inline void assign(const _BasicJsonTy &json, float_type &value)
        {
            if (!json.is_float())
                throw json_type_error("json value type must be float");
            value = json.value_.data.number_float;
        }

        template <
            typename _FloatingTy,
            typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
        static inline void assign(const _BasicJsonTy &json, _FloatingTy &value)
        {
            if (!json.is_float())
                throw json_type_error("json value type must be float");
            value = static_cast<_FloatingTy>(json.value_.data.number_float);
        }
    };
} // namespace jsonxx
