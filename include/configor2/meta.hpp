// Copyright (c) 2018-2021 configor - Nomango
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
#include <type_traits> // std::is_same, std::integral_constant, std::remove_cv, std::remove_reference

namespace configor {

namespace detail {

// nonesuch

struct nonesuch {
    nonesuch()                 = delete;
    ~nonesuch()                = delete;
    nonesuch(nonesuch const&)  = delete;
    nonesuch(nonesuch const&&) = delete;
    void operator=(nonesuch const&) = delete;
    void operator=(nonesuch&&) = delete;
};

// priority

template <int P>
struct priority : priority<P - 1> {};

template <>
struct priority<0> {};

// remove_cvref

template <class T>
struct remove_cvref {
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

// always_void

template <class T>
struct always_void {
    using type = void;
};

// exact_detect

template <class _Void, template <class...> class Fn, class... Args>
struct detect_impl {
    struct dummy {
        dummy()             = delete;
        ~dummy()            = delete;
        dummy(const dummy&) = delete;
    };

    using type = dummy;

    static constexpr bool value = false;
};

template <template <class...> class Fn, class... Args>
struct detect_impl<typename always_void<Fn<Args...>>::type, Fn, Args...> {
    using type = Fn<Args...>;

    static constexpr bool value = true;
};

template <class _Expected, template <class...> class Fn, class... Args>
using exact_detect = std::is_same<_Expected, typename detect_impl<void, Fn, Args...>::type>;

template <template <class...> class Fn, class... Args>
using is_detected = std::integral_constant<bool, detect_impl<void, Fn, Args...>::value>;

// static_const

template <class T>
struct static_const {
    static constexpr T value = {};
};

template <class T>
constexpr T static_const<T>::value;

} // namespace detail

} // namespace configor
