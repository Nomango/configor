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
#include <stdexcept>

namespace jsonxx
{
    //
    // exceptions
    //

    class json_exception
        : public std::runtime_error
    {
    public:
        json_exception(const char *message)
            : std::runtime_error(message)
        {
        }
    };

    class json_type_error
        : public json_exception
    {
    public:
        json_type_error(const char *message) : json_exception(message) {}
    };

    class json_invalid_key
        : public json_exception
    {
    public:
        json_invalid_key(const char *message) : json_exception(message) {}
    };

    class json_invalid_iterator
        : public json_exception
    {
    public:
        json_invalid_iterator(const char *message) : json_exception(message) {}
    };

    class json_parse_error
        : public json_exception
    {
    public:
        json_parse_error(const char *message) : json_exception(message) {}
    };

    class json_serialize_error
        : public json_exception
    {
    public:
        json_serialize_error(const char *message) : json_exception(message) {}
    };
} // namespace jsonxx
