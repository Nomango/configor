#pragma once

#include "configor_meta.hpp"

#include <cmath>    // std::fabs
#include <cstddef>  // std::ptrdiff_t, std::size_t
#include <limits>   // std::numeric_limits
#include <memory>   // std::allocator_traits

namespace configor
{

namespace detail
{
template <typename T>
bool nearly_equal(T a, T b)
{
    return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
}
}  // namespace detail

template <class TplArgs>
class basic_configor final
{
public:
    using boolean_type = typename TplArgs::boolean_type;
    using integer_type = typename TplArgs::integer_type;
    using float_type   = typename TplArgs::float_type;
    using char_type    = typename TplArgs::char_type;
    using string_type  = typename TplArgs::template string_type<char_type>;
    using array_type   = typename TplArgs::template array_type<basic_configor>;
    using object_type  = typename TplArgs::template object_type<string_type, basic_configor>;

    template <typename T>
    using allocator_type = typename TplArgs::template allocator_type<T>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    struct data_t
    {
        configor_type type = configor_type::null;
        union value_t
        {
            boolean_type boolean;
            integer_type number_integer;
            float_type   number_float;
            string_type* string;
            array_type*  array;
            object_type* object;
        } value = {};
    } inner_data;

public:
    basic_configor(std::nullptr_t = nullptr) {}

    basic_configor(const basic_configor& other)
    {
        inner_data.type = other.inner_data.type;
        switch (other.inner_data.type)
        {
        case configor_type::object:
            inner_data.value.object = create_value<object_type>(*other.inner_data.value.object);
            break;
        case configor_type::array:
            inner_data.value.array = create_value<array_type>(*other.inner_data.value.array);
            break;
        case configor_type::string:
            inner_data.value.string = create_value<string_type>(*other.inner_data.value.string);
            break;
        case configor_type::number_integer:
            inner_data.value.number_integer = other.inner_data.value.number_integer;
            break;
        case configor_type::number_float:
            inner_data.value.number_float = other.inner_data.value.number_float;
            break;
        case configor_type::boolean:
            inner_data.value.boolean = other.inner_data.value.boolean;
            break;
        default:
            break;
        }
    }

    basic_configor(basic_configor&& other)
        : inner_data{ other.inner_data }
    {
        other.inner_data.type         = configor_type::null;
        other.inner_data.value.object = nullptr;
    }

    basic_configor(const string_type& s)
    {
        inner_data.type         = configor_type::string;
        inner_data.value.string = create_value<string_type>(s);
    }

    ~basic_configor()
    {
        destroy();
    }

    inline data_t& data()
    {
        return inner_data;
    }

    inline const data_t& data() const
    {
        return inner_data;
    }

    inline configor_type type() const
    {
        return inner_data.type;
    }

    inline bool is_null() const
    {
        return type() == configor_type::null;
    }

    inline bool is_boolean() const
    {
        return type() == configor_type::boolean;
    }

    inline bool is_integer() const
    {
        return type() == configor_type::number_integer;
    }

    inline bool is_float() const
    {
        return type() == configor_type::number_float;
    }

    inline bool is_number() const
    {
        return is_integer() || is_float();
    }

    inline bool is_string() const
    {
        return type() == configor_type::string;
    }

    inline bool is_array() const
    {
        return type() == configor_type::array;
    }

    inline bool is_object() const
    {
        return type() == configor_type::object;
    }

    // Clears the content and keeps the type.
    void clear()
    {
        switch (type())
        {
        case configor_type::object:
            inner_data.value.object->clear();
            break;
        case configor_type::array:
            inner_data.value.array->clear();
            break;
        case configor_type::string:
            inner_data.value.string->clear();
            break;
        case configor_type::number_integer:
            inner_data.value.number_integer = 0;
            break;
        case configor_type::number_float:
            inner_data.value.number_float = 0;
            break;
        case configor_type::boolean:
            inner_data.value.boolean = false;
            break;
        }
    }

    // Destroys inner_data and resets type to null.
    void destroy()
    {
        switch (inner_data.type)
        {
        case configor_type::object:
            destroy_value<object_type>(inner_data.value.object);
            break;
        case configor_type::array:
            destroy_value<array_type>(inner_data.value.array);
            break;
        case configor_type::string:
            destroy_value<string_type>(inner_data.value.string);
            break;
        }
        inner_data.type = configor_type::null;
    }

private:
    template <typename T, typename... VArgs>
    T* create_value(VArgs&&... args)
    {
        using allocator_traits = std::allocator_traits<allocator_type<T>>;

        auto& alloc = get_allocator<T>();

        T* ptr = allocator_traits::allocate(alloc, 1);
        allocator_traits::construct(alloc, ptr, std::forward<VArgs>(args)...);
        return ptr;
    }

    template <typename T>
    void destroy_value(T* ptr)
    {
        using allocator_traits = std::allocator_traits<allocator_type<T>>;

        auto& alloc = get_allocator<T>();

        allocator_traits::destroy(alloc, ptr);
        allocator_traits::deallocate(alloc, ptr, 1);
    }

    template <typename T>
    static inline allocator_type<T>& get_allocator()
    {
        static allocator_type<T> allocator;
        return allocator;
    }
};

}  // namespace configor
