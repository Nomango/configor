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
#include <cassert>    // assert
#include <stdexcept>  // std::runtime_error, std::exception_ptr, std::rethrow_exception
#include <string>     // std::string

#ifndef JSONXX_ASSERT
#define JSONXX_ASSERT(...) assert(__VA_ARGS__)
#endif

namespace jsonxx
{

//
// exceptions
//

class json_exception : public std::runtime_error
{
public:
    json_exception(const char* message)
        : std::runtime_error(message)
    {
    }

    json_exception(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class json_type_error : public json_exception
{
public:
    json_type_error(const std::string& message)
        : json_exception("json type error: " + message)
    {
    }
};

class json_invalid_key : public json_exception
{
public:
    json_invalid_key(const std::string& message)
        : json_exception("invalid json key error: " + message)
    {
    }
};

class json_invalid_iterator : public json_exception
{
public:
    json_invalid_iterator(const std::string& message)
        : json_exception("invalid json iterator error: " + message)
    {
    }
};

class json_deserialization_error : public json_exception
{
public:
    json_deserialization_error(const std::string& message)
        : json_exception("json deserialization error: " + message)
    {
    }
};

class json_serialization_error : public json_exception
{
public:
    json_serialization_error(const std::string& message)
        : json_exception("json serialization error: " + message)
    {
    }
};

//
// error handler
//

enum class error_policy
{
    strict = 1,  // throw exceptions
    record = 2,  // record error message
    ignore = 3,  // ignore all errors
};

class error_handler
{
public:
    virtual void handle(std::exception_ptr eptr) = 0;
};

template <error_policy _Policy>
class error_handler_with;

template <>
class error_handler_with<error_policy::strict> : public error_handler
{
public:
    virtual void handle(std::exception_ptr eptr) override
    {
        std::rethrow_exception(eptr);
    }
};

template <>
class error_handler_with<error_policy::ignore> : public error_handler
{
public:
    virtual void handle(std::exception_ptr eptr) override
    {
        // DO NOTHING
    }
};

template <>
class error_handler_with<error_policy::record> : public error_handler
{
public:
    virtual void handle(std::exception_ptr eptr) override
    {
        try
        {
            if (eptr)
            {
                std::rethrow_exception(eptr);
            }
        }
        catch (const json_exception& e)
        {
            this->error = e.what();
        }
    }

    std::string error;
};

}  // namespace jsonxx
