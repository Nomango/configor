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
#include <stdexcept> // std::runtime_error, std::exception_ptr, std::rethrow_exception
#include <string>    // std::string

#ifndef CONFIGOR_ASSERT
#include <cassert> // assert
#define CONFIGOR_ASSERT(...) assert(__VA_ARGS__)
#endif

namespace configor {

namespace detail {

class exception : public std::runtime_error {
public:
    explicit exception(const char* message) : std::runtime_error(message) {}

    explicit exception(const std::string& message) : std::runtime_error(message) {}
};

class bad_cast : public exception {
public:
    explicit bad_cast(const std::string& message) : exception("bad cast: " + message) {}
};

class invalid_argument : public exception {
public:
    explicit invalid_argument(const std::string& message) : exception("invalid argument: " + message) {}
};

class iterator_error : public exception {
public:
    explicit iterator_error(const std::string& message) : exception("iterator error: " + message) {}
};

class bad_serialization : public exception {
public:
    explicit bad_serialization(const std::string& message) : exception("bad serialization: " + message) {}
};

class bad_deserialization : public exception {
public:
    explicit bad_deserialization(const std::string& message) : exception("bad deserialization: " + message) {}
};

} // namespace detail

//
// error handler
//

enum class error_policy {
    strict = 1, // throw exceptions
    record = 2, // record error message
    ignore = 3, // ignore all errors
};

class error_handler {
public:
    virtual void handle(std::exception_ptr eptr) = 0;
};

template <error_policy _Policy>
class error_handler_with;

template <>
class error_handler_with<error_policy::strict> : public error_handler {
public:
    virtual void handle(std::exception_ptr eptr) override { std::rethrow_exception(eptr); }
};

template <>
class error_handler_with<error_policy::ignore> : public error_handler {
public:
    virtual void handle(std::exception_ptr eptr) override {
        // DO NOTHING
    }
};

template <>
class error_handler_with<error_policy::record> : public error_handler {
public:
    virtual void handle(std::exception_ptr eptr) override {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const detail::exception& e) {
            this->error = e.what();
        }
    }

    std::string error;
};

} // namespace configor
