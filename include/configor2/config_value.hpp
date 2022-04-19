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
#include "config_declare.hpp"
#include "config_iterator.hpp"

#include <algorithm> // std::swap
#include <utility>   // std::forward, std::declval

namespace configor {
namespace detail {

inline bad_cast make_conversion_error(config_value_type actual, config_value_type expect) {
    return bad_cast(std::string("cannot convert type '") + to_string(actual) + "' to type '" + to_string(expect) +
                    "' (implicitly)");
}

template <typename ValueArgs>
class config_value {
    friend struct detail::iterator<config_value>;
    friend struct detail::iterator<const config_value>;

public:
    using boolean_type = typename ValueArgs::boolean_type;
    using char_type    = typename ValueArgs::char_type;
    using integer_type = typename ValueArgs::integer_type;
    using float_type   = typename ValueArgs::float_type;
    using string_type =
        typename ValueArgs::template string_type<char_type, std::char_traits<char_type>,
                                                 typename ValueArgs::template allocator_type<char_type>>;
    using array_type =
        typename ValueArgs::template array_type<config_value,
                                                typename ValueArgs::template allocator_type<config_value>>;
    using object_type = typename ValueArgs::template object_type<
        string_type, config_value, std::less<string_type>,
        typename ValueArgs::template allocator_type<std::pair<const string_type, config_value>>>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using iterator               = detail::iterator<config_value>;
    using const_iterator         = detail::iterator<const config_value>;
    using reverse_iterator       = detail::reverse_iterator<iterator>;
    using const_reverse_iterator = detail::reverse_iterator<const_iterator>;

public:
    config_value() : type_(config_value_type::null), data_{} {}

    config_value(const config_value_type type) { init(type); }

    config_value(const config_value& other) : type_(other.type_) {
        switch (other.type_) {
        case config_value_type::object:
            data_.object = create<object_type>(*other.data_.object);
            break;
        case config_value_type::array:
            data_.array = create<array_type>(*other.data_.array);
            break;
        case config_value_type::string:
            data_.string = create<string_type>(*other.data_.string);
            break;
        case config_value_type::number_integer:
            data_.number_integer = other.data_.number_integer;
            break;
        case config_value_type::number_float:
            data_.number_float = other.data_.number_float;
            break;
        case config_value_type::boolean:
            data_.boolean = other.data_.boolean;
            break;
        default:
            data_ = {};
            break;
        }
    }

    config_value(config_value&& other) noexcept : type_(other.type_), data_(other.data_) {
        other.type_ = config_value_type::null;
        other.data_ = {};
    }

    // TODO
    // template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
    //           typename std::enable_if<!is_config<_UTy>::value && detail::has_to_config<config_value, _UTy>::value,
    //                                   int>::type = 0>
    // config_value(_CompatibleTy&& value) {
    //     binder_type<_UTy>::to_config(*this, std::forward<_CompatibleTy>(value));
    // }

    // TODO object
    // TODO array

    ~config_value() {
        switch (type_) {
        case config_value_type::object:
            destroy<object_type>(data_.object);
            break;
        case config_value_type::array:
            destroy<array_type>(data_.array);
            break;
        case config_value_type::string:
            destroy<string_type>(data_.string);
            break;
        }
    }

    inline config_value_type type() const { return type_; }

    inline const char* type_name() const { return to_string(type()); }

    inline bool is_null() const { return type_ == config_value_type::null; }

    inline bool is_bool() const { return type_ == config_value_type::boolean; }

    inline bool is_integer() const { return type_ == config_value_type::number_integer; }

    inline bool is_float() const { return type_ == config_value_type::number_float; }

    inline bool is_number() const { return is_integer() || is_float(); }

    inline bool is_string() const { return type_ == config_value_type::string; }

    inline bool is_array() const { return type_ == config_value_type::array; }

    inline bool is_object() const { return type_ == config_value_type::object; }

public:
    iterator begin() {
        iterator iter(this);
        iter.set_begin();
        return iter;
    }

    inline const_iterator begin() const { return cbegin(); }

    inline const_iterator cbegin() const {
        const_iterator iter(this);
        iter.set_begin();
        return iter;
    }

    iterator end() {
        iterator iter(this);
        iter.set_end();
        return iter;
    }

    inline const_iterator end() const { return cend(); }

    const_iterator cend() const {
        const_iterator iter(this);
        iter.set_end();
        return iter;
    }

    inline reverse_iterator rbegin() { return reverse_iterator(end()); }

    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    inline const_reverse_iterator crbegin() const { return rbegin(); }

    inline reverse_iterator rend() { return reverse_iterator(begin()); }

    inline const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    inline const_reverse_iterator crend() const { return rend(); }

public:
    size_type size() const {
        switch (type()) {
        case config_value_type::null:
            return 0;
        case config_value_type::array:
            return data_.array->size();
        case config_value_type::object:
            return data_.object->size();
        default:
            return 1;
        }
    }

    bool empty() const {
        if (is_null())
            return true;

        if (is_object())
            return data_.object->empty();

        if (is_array())
            return data_.array->empty();

        return false;
    }

    iterator find(const typename object_type::key_type& key) {
        if (is_object()) {
            iterator iter(this);
            iter.object_it_ = data_.object->find(key);
            return iter;
        }
        return end();
    }

    const_iterator find(const typename object_type::key_type& key) const {
        if (is_object()) {
            const_iterator iter(this);
            iter.object_it_ = data_.object->find(key);
            return iter;
        }
        return cend();
    }

    inline size_type count(const typename object_type::key_type& key) const {
        return is_object() ? data_.object->count(key) : 0;
    }

    size_type erase(const typename object_type::key_type& key) {
        if (!is_object()) {
            throw invalid_argument("cannot use erase() with non-object value");
        }
        return data_.object->erase(key);
    }

    void erase(const size_type index) {
        if (!is_array()) {
            throw invalid_argument("cannot use erase() with non-array value");
        }
        data_.array->erase(data_.array->begin() + static_cast<difference_type>(index));
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value ||
                                                                std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy pos) {
        _IterTy result = end();

        switch (type()) {
        case config_value_type::object: {
            result.object_it_ = data_.object->erase(pos.object_it_);
            break;
        }

        case config_value_type::array: {
            result.array_it_ = data_.array->erase(pos.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value ||
                                                                std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy first, _IterTy last) {
        _IterTy result = end();

        switch (type()) {
        case config_value_type::object: {
            result.object_it_ = data_.object->erase(first.object_it_, last.object_it_);
            break;
        }

        case config_value_type::array: {
            result.array_it_ = data_.array->erase(first.array_it_, last.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    void push_back(config_value&& config) {
        if (!is_null() && !is_array()) {
            throw configor_type_error("cannot use push_back() with non-array value");
        }

        if (is_null()) {
            init(config_value_type::array);
        }

        data_.array->push_back(std::move(config));
    }

    void clear() {
        switch (type_) {
        case config_value_type::number_integer:
            data_.number_integer = 0;
            break;
        case config_value_type::number_float:
            data_.number_float = static_cast<float_type>(0.0);
            break;
        case config_value_type::boolean:
            data_.boolean = false;
            break;
        case config_value_type::string:
            data_.string->clear();
            break;
        case config_value_type::array:
            data_.array->clear();
            break;
        case config_value_type::object:
            data_.object->clear();
            break;
        }
    }

    friend void swap(config_value& lhs, config_value& rhs) {
        std::swap(lhs.type_, rhs.type_);
        std::swap(lhs.data_, rhs.data_);
    }

private:
    // get pointer value

    inline boolean_type* do_get_ptr(boolean_type*) noexcept { return is_bool() ? &data_.boolean : nullptr; }

    inline const boolean_type* do_get_ptr(const boolean_type*) const noexcept {
        return is_bool() ? &data_.boolean : nullptr;
    }

    inline integer_type* do_get_ptr(integer_type*) noexcept { return is_integer() ? &data_.number_integer : nullptr; }

    inline const integer_type* do_get_ptr(const integer_type*) const noexcept {
        return is_integer() ? &data_.number_integer : nullptr;
    }

    inline float_type* do_get_ptr(float_type*) noexcept { return is_float() ? &data_.number_float : nullptr; }

    inline const float_type* do_get_ptr(const float_type*) const noexcept {
        return is_float() ? &data_.number_float : nullptr;
    }

    inline string_type* do_get_ptr(string_type*) noexcept { return is_string() ? data_.string : nullptr; }

    inline const string_type* do_get_ptr(const string_type*) const noexcept {
        return is_string() ? data_.string : nullptr;
    }

    inline array_type* do_get_ptr(array_type*) noexcept { return is_array() ? data_.array : nullptr; }

    inline const array_type* do_get_ptr(const array_type*) const noexcept { return is_array() ? data_.array : nullptr; }

    inline object_type* do_get_ptr(object_type*) noexcept { return is_object() ? data_.object : nullptr; }

    inline const object_type* do_get_ptr(const object_type*) const noexcept {
        return is_object() ? data_.object : nullptr;
    }

public:
    // get_ptr

    template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
    auto get_ptr() const -> decltype(std::declval<const config_value&>().do_get_ptr(std::declval<T>())) {
        return do_get_ptr(static_cast<T>(nullptr));
    }

    template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
    auto get_ptr() -> decltype(std::declval<config_value&>().do_get_ptr(std::declval<T>())) {
        return do_get_ptr(static_cast<T>(nullptr));
    }

private:
    // get reference value
    template <typename _RefTy, typename Config, typename _PtrTy = typename std::add_pointer<_RefTy>::type>
    static auto do_get_ref(Config& c) -> decltype(c.template get_ptr<_PtrTy>(), std::declval<_RefTy>()) {
        auto* ptr = c.template get_ptr<_PtrTy>();
        if (ptr != nullptr) {
            return *ptr;
        }
        throw configor_type_error("incompatible reference type for get, actual type is " + std::string(c.type_name()));
    }

    // get value

    template <typename T, typename = typename std::enable_if<std::is_same<config_value, T>::value>::type>
    T do_get(detail::priority<3>) const {
        return *this;
    }

    template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
    auto do_get(detail::priority<2>) const noexcept
        -> decltype(std::declval<const config_value&>().do_get_ptr(std::declval<T>())) {
        return do_get_ptr(static_cast<T>(nullptr));
    }

    template <typename T,
              typename = typename std::enable_if<detail::has_non_default_from_config<config_value, T>::value>::type>
    T do_get(detail::priority<1>) const {
        return binder_type<T>::from_config(*this);
    }

    template <typename T, typename = typename std::enable_if<std::is_default_constructible<T>::value &&
                                                             detail::has_from_config<config_value, T>::value>::type>
    T do_get(detail::priority<0>) const {
        T value{};
        binder_type<T>::from_config(*this, value);
        return value;
    }

    // get value by reference

    template <typename T, typename = typename std::enable_if<std::is_same<config_value, T>::value>::type>
    T& do_get(T& value, detail::priority<3>) const {
        return (value = *this);
    }

    template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
    auto do_get(T& value, detail::priority<2>) const noexcept
        -> decltype(std::declval<const config_value&>().do_get_ptr(std::declval<T>())) {
        return (value = do_get_ptr(static_cast<T>(nullptr)));
    }

    template <typename T, typename = typename std::enable_if<detail::has_from_config<config_value, T>::value>::type>
    T& do_get(T& value, detail::priority<1>) const {
        binder_type<T>::from_config(*this, value);
        return value;
    }

    template <typename T,
              typename = typename std::enable_if<detail::has_non_default_from_config<config_value, T>::value>::type>
    T& do_get(T& value, detail::priority<0>) const {
        value = binder_type<T>::from_config(*this);
        return value;
    }

public:
    // get

    template <typename T, typename _UTy = typename detail::remove_cvref<T>::type,
              typename = typename std::enable_if<!std::is_reference<T>::value>::type>
    auto get() const -> decltype(std::declval<const config_value&>().template do_get<_UTy>(detail::priority<3>{})) {
        return do_get<_UTy>(detail::priority<3>{});
    }

    template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
    auto get() -> decltype(std::declval<config_value&>().template get_ptr<T>()) {
        return get_ptr<T>();
    }

    template <typename T, typename std::enable_if<std::is_reference<T>::value, int>::type = 0>
    auto get() -> decltype(config_value::template do_get_ref<T>(std::declval<config_value&>())) {
        return do_get_ref<T>(*this);
    }

    template <typename T, typename std::enable_if<std::is_reference<T>::value &&
                                                      std::is_const<typename std::remove_reference<T>::type>::value,
                                                  int>::type = 0>
    auto get() const -> decltype(config_value::template do_get_ref<T>(std::declval<const config_value&>())) {
        return do_get_ref<T>(*this);
    }

    template <typename T>
    auto get(T& value) const
        -> decltype(std::declval<const config_value&>().template do_get<T>(std::declval<T&>(), detail::priority<3>{}),
                    bool{}) {
        try {
            do_get<T>(value, detail::priority<3>{});
            return true;
        } catch (...) {
        }
        return false;
    }

    template <typename T,
              typename = typename std::enable_if<detail::is_configor_getable<config_value, T>::value &&
                                                 !std::is_pointer<T>::value && !std::is_reference<T>::value>::type>
    inline operator T() const {
        return get<T>();
    }

    boolean_type as_bool() const {
        switch (type()) {
        case config_value_type::number_integer:
            return data_.number_integer != 0;
        case config_value_type::number_float:
            return data_.number_float != 0;
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
            return !empty();
        case config_value_type::boolean:
            return data_.boolean;
        case config_value_type::null:
            break;
        }
        return false;
    }

    integer_type as_integer() const {
        switch (type()) {
        case config_value_type::number_integer:
            return data_.number_integer;
        case config_value_type::number_float:
            return static_cast<integer_type>(data_.number_float);
        case config_value_type::boolean:
            return data_.boolean ? integer_type(1) : integer_type(0);
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
        case config_value_type::null:
            throw detail::make_conversion_error(type(), config_value_type::number_integer);
        }
        return 0;
    }

    float_type as_float() const {
        switch (type()) {
        case config_value_type::number_integer:
            return static_cast<float_type>(data_.number_integer);
        case config_value_type::number_float:
            return data_.number_float;
        case config_value_type::boolean:
            return data_.boolean ? float_type(1) : float_type(0);
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
        case config_value_type::null:
            throw detail::make_conversion_error(type(), config_value_type::number_integer);
        }
        return 0;
    }

    string_type as_string() const {
        switch (type()) {
        case config_value_type::string:
            return *data_.string;
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

    config_value& operator=(const config_value& rhs) {
        if (this != &rhs) {
            swap(*this, config_value{rhs});
        }
        return *this;
    }

    config_value& operator=(config_value&& rhs) {
        if (this != &rhs) {
            swap(*this, config_value{});
            swap(*this, rhs);
        }
        return *this;
    }

public:
    // operator[] functions

    inline config_value& operator[](const size_type index) {
        if (is_null()) {
            init(config_value_type::array);
        }

        if (!is_array()) {
            throw invalid_argument("operator[] called on a non-array object");
        }

        if (index >= data_.array->size()) {
            data_.array->insert(data_.array->end(), index - data_.array->size() + 1, config_value());
        }
        return (*data_.array)[index];
    }

    inline const config_value& operator[](const size_type index) const { return at(index); }

    inline config_value& operator[](const typename object_type::key_type& key) {
        if (is_null()) {
            init(config_value_type::object);
        }

        if (!is_object()) {
            throw invalid_argument("operator[] called on a non-object type");
        }
        return (*data_.object)[key];
    }

    inline const config_value& operator[](const typename object_type::key_type& key) const { return at(key); }

    inline const config_value& at(const size_type index) const {
        if (!is_array()) {
            throw invalid_argument("operator[] called on a non-array type");
        }

        if (index >= data_.array->size()) {
            throw std::out_of_range("operator[] index out of range");
        }
        return (*data_.array)[index];
    }

    template <typename CharT>
    inline config_value& operator[](CharT* key) {
        if (is_null()) {
            init(config_value_type::object);
        }

        if (!is_object()) {
            throw invalid_argument("operator[] called on a non-object object");
        }
        return (*data_.object)[key];
    }

    template <typename CharT>
    inline const config_value& operator[](CharT* key) const {
        return at(key);
    }

    inline config_value& at(const typename object_type::key_type& key) const {
        if (!is_object()) {
            throw invalid_argument("operator[] called on a non-object object");
        }

        auto iter = data_.object->find(key);
        if (iter == data_.object->end()) {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

    template <typename CharT>
    inline const config_value& at(CharT* key) const {
        if (!is_object()) {
            throw invalid_argument("operator[] called on a non-object object");
        }

        auto iter = data_.object->find(key);
        if (iter == data_.object->end()) {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

public:
    // eq functions

    friend bool operator==(const config_value& lhs, const config_value& rhs) {
        if (lhs.type_ == rhs.type_) {
            switch (lhs.type_) {
            case config_value_type::array:
                return (*lhs.data_.vector == *rhs.data_.vector);
            case config_value_type::object:
                return (*lhs.data_.object == *rhs.data_.object);
            case config_value_type::null:
                return true;
            case config_value_type::string:
                return (*lhs.data_.string == *rhs.data_.string);
            case config_value_type::boolean:
                return (lhs.data_.boolean == rhs.data_.boolean);
            case config_value_type::number_integer:
                return (lhs.data_.number_integer == rhs.data_.number_integer);
            case config_value_type::number_float:
                return detail::nearly_equal(lhs.data_.number_float, rhs.data_.number_float);
            default:
                return false;
            }
        } else if (lhs.type_ == config_value_type::number_integer && rhs.type_ == config_value_type::number_float) {
            return detail::nearly_equal<float_type>(static_cast<float_type>(lhs.data_.number_integer),
                                                    rhs.data_.number_float);
        } else if (lhs.type_ == config_value_type::number_float && rhs.type_ == config_value_type::number_integer) {
            return detail::nearly_equal<float_type>(lhs.data_.number_float,
                                                    static_cast<float_type>(rhs.data_.number_integer));
        }
        return false;
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator==(const config_value& lhs, ScalarT rhs) {
        return lhs == config_value(rhs);
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator==(ScalarT lhs, const config_value& rhs) {
        return config_value(lhs) == rhs;
    }

    // ne functions

    friend inline bool operator!=(const config_value& lhs, const config_value& rhs) { return !(lhs == rhs); }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator!=(const config_value& lhs, ScalarT rhs) {
        return lhs != config_value(rhs);
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator!=(ScalarT lhs, const config_value& rhs) {
        return config_value(lhs) != rhs;
    }

    // lt functions

    friend bool operator<(const config_value& lhs, const config_value& rhs) {
        const auto lhs_type = lhs.type();
        const auto rhs_type = rhs.type();

        if (lhs_type == rhs_type) {
            switch (lhs_type) {
            case config_value_type::array:
                return (*lhs.data_.array) < (*rhs.data_.array);
            case config_value_type::object:
                return (*lhs.data_.object) < (*rhs.data_.object);
            case config_value_type::null:
                return false;
            case config_value_type::string:
                return (*lhs.data_.string) < (*rhs.data_.string);
            case config_value_type::boolean:
                return (lhs.data_.boolean < rhs.data_.boolean);
            case config_value_type::number_integer:
                return (lhs.data_.number_integer < rhs.data_.number_integer);
            case config_value_type::number_float:
                return (lhs.data_.number_float < rhs.data_.number_float);
            default:
                return false;
            }
        } else if (lhs_type == config_value_type::number_integer && rhs_type == config_value_type::number_float) {
            return (static_cast<float_type>(lhs.data_.number_integer) < rhs.data_.number_float);
        } else if (lhs_type == config_value_type::number_float && rhs_type == config_value_type::number_integer) {
            return (lhs.data_.number_float < static_cast<float_type>(rhs.data_.number_integer));
        }

        return false;
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator<(const config_value& lhs, ScalarT rhs) {
        return lhs < config_value(rhs);
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator<(ScalarT lhs, const config_value& rhs) {
        return config_value(lhs) < rhs;
    }

    // lte functions

    friend inline bool operator<=(const config_value& lhs, const config_value& rhs) { return !(rhs < lhs); }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator<=(const config_value& lhs, ScalarT rhs) {
        return lhs <= config_value(rhs);
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator<=(ScalarT lhs, const config_value& rhs) {
        return config_value(lhs) <= rhs;
    }

    // gt functions

    friend inline bool operator>(const config_value& lhs, const config_value& rhs) { return rhs < lhs; }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator>(const config_value& lhs, ScalarT rhs) {
        return lhs > config_value(rhs);
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator>(ScalarT lhs, const config_value& rhs) {
        return config_value(lhs) > rhs;
    }

    // gte functions

    friend inline bool operator>=(const config_value& lhs, const config_value& rhs) { return !(lhs < rhs); }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator>=(const config_value& lhs, ScalarT rhs) {
        return lhs >= config_value(rhs);
    }

    template <typename ScalarT, typename = typename std::enable_if<std::is_scalar<ScalarT>::value>::type>
    friend inline bool operator>=(ScalarT lhs, const config_value& rhs) {
        return config_value(lhs) >= rhs;
    }

public:
    const auto& raw_value() const { return data_; }

    auto& raw_value() { return data_; }

private:
    void init(const config_value_type type) {
        switch (type) {
        case config_value_type::object:
            data_.object = create<object_type>();
            break;
        case config_value_type::array:
            data_.array = create<array_type>();
            break;
        case config_value_type::string:
            data_.string = create<string_type>();
            break;
        case config_value_type::number_integer:
            data_.number_integer = integer_type{};
            break;
        case config_value_type::number_float:
            data_.number_float = float_type{};
            break;
        case config_value_type::boolean:
            data_.boolean = boolean_type{};
            break;
        default:
            data_ = {};
            break;
        }
    }

    template <typename T, typename... Args>
    inline T* create(Args&&... args) {
        using allocator_type   = typename ValueArgs::template allocator_type<T>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        allocator_type allocator{};

        auto* ptr = allocator_traits::allocate(allocator, 1);
        allocator_traits::construct(allocator, ptr, std::forward<Args>(args)...);
        return ptr;
    }

    template <typename T>
    inline void destroy(T* ptr) {
        using allocator_type   = typename ValueArgs::template allocator_type<T>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        allocator_type allocator{};
        allocator_traits::destroy(allocator, ptr);
        allocator_traits::deallocate(allocator, ptr, 1);
    }

private:
    config_value_type type_;
    union {
        boolean_type boolean;
        integer_type number_integer;
        float_type   number_float;
        string_type* string;
        object_type* object;
        array_type*  array;
    } data_;
};
} // namespace detail
} // namespace configor
