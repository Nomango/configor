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

#include <array>        // std::array
#include <cstdint>      // uint32_t
#include <string>       // std::string, std::wstring, std::u16string, std::u32string
#include <type_traits>  // std::char_traits, std::true_type, std::false_type
#include <vector>       // std::vector

// unicode constants
#define JSONXX_UNICODE_SUR_BASE 0x10000
#define JSONXX_UNICODE_SUR_LEAD_BEGIN 0xD800
#define JSONXX_UNICODE_SUR_LEAD_END 0xDBFF
#define JSONXX_UNICODE_SUR_TRAIL_BEGIN 0xDC00
#define JSONXX_UNICODE_SUR_TRAIL_END 0xDFFF
#define JSONXX_UNICODE_SUR_BITS 10
#define JSONXX_UNICODE_SUR_MAX 0x3FF

namespace jsonxx
{
namespace detail
{

inline uint32_t merge_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate)
{
    uint32_t code = ((lead_surrogate - JSONXX_UNICODE_SUR_LEAD_BEGIN) << JSONXX_UNICODE_SUR_BITS);
    code += (trail_surrogate - JSONXX_UNICODE_SUR_TRAIL_BEGIN);
    code += JSONXX_UNICODE_SUR_BASE;
    return code;
}

//
// unicode_reader
//

template <typename _StrTy>
struct unicode_reader;

template <>
struct unicode_reader<std::string>
{
    using string_type = std::string;

    const string_type& val;
    const bool         escape_unicode;
    uint8_t            state = 0;

    unicode_reader(const string_type& val, const bool escape_unicode)
        : val(val)
        , escape_unicode(escape_unicode)
    {
    }

    inline uint8_t get_byte(size_t i) const
    {
        return static_cast<uint8_t>(val.at(i));
    }

    bool get_code(size_t& i, uint32_t& code_output)
    {
        // Unicode              UTF-8
        // U+0000...U+007F      0xxxxxxx
        // U+0080...U+07FF      110xxxxx 10xxxxxx
        // U+0800...U+FFFF      1110xxxx 10xxxxxx 10xxxxxx
        // U+10000...U+10FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        static const std::array<std::uint8_t, 256> utf8_extra_bytes = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
        };

        static const std::array<std::uint32_t, 6> utf8_offsets = {
            0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080,
        };

        if (i >= val.size())
        {
            return false;
        }

        if (!escape_unicode)
        {
            // code point will not be escaped
            const auto byte = get_byte(i);
            code_output     = static_cast<uint32_t>(byte);
            i++;
            return true;
        }

        const auto first_byte          = get_byte(i);
        const auto extra_bytes_to_read = utf8_extra_bytes[first_byte];
        if ((i + static_cast<size_t>(extra_bytes_to_read)) >= val.size())
        {
            throw json_serialize_error("string was incomplete");
        }

        // read bytes
        uint32_t code = 0;
        switch (extra_bytes_to_read)
        {
        case 5:
            code += static_cast<uint32_t>(get_byte(i));
            i++;
            code <<= 6;
        case 4:
            code += static_cast<uint32_t>(get_byte(i));
            i++;
            code <<= 6;
        case 3:
            code += static_cast<uint32_t>(get_byte(i));
            i++;
            code <<= 6;
        case 2:
            code += static_cast<uint32_t>(get_byte(i));
            i++;
            code <<= 6;
        case 1:
            code += static_cast<uint32_t>(get_byte(i));
            i++;
            code <<= 6;
        case 0:
            code += static_cast<uint32_t>(get_byte(i));
            i++;
        }
        code -= utf8_offsets[extra_bytes_to_read];

        code_output = code;
        return true;
    }
};

template <>
struct unicode_reader<std::wstring>
{
    using string_type = std::wstring;
    using char_type   = typename string_type::value_type;

    const string_type& val;
    const bool         escape_unicode;

    unicode_reader(const string_type& val, const bool escape_unicode)
        : val(val)
        , escape_unicode(escape_unicode)
    {
    }

    bool get_code(size_t& i, uint32_t& code)
    {
        return get_code(i, code, std::integral_constant<bool, sizeof(char_type) == 4>());
    }

    bool get_code(size_t& i, uint32_t& code, std::true_type /* sizeof(wchar_t) == 4 */)
    {
        if (i >= val.size())
        {
            return false;
        }
        code = static_cast<uint32_t>(val.at(i));
        i++;
        return true;
    }

    bool get_code(size_t& i, uint32_t& code, std::false_type /* sizeof(wchar_t) == 2 */)
    {
        if (i >= val.size())
        {
            return false;
        }

        code = static_cast<uint32_t>(static_cast<uint16_t>(val.at(i)));
        i++;

        if (!escape_unicode)
        {
            // code point will not be escaped
            return true;
        }

        if (JSONXX_UNICODE_SUR_LEAD_BEGIN <= code && code <= JSONXX_UNICODE_SUR_LEAD_END)
        {
            if (i >= val.size())
            {
                throw json_serialize_error("string was incomplete");
            }

            uint32_t lead_surrogate  = code;
            uint32_t trail_surrogate = static_cast<uint32_t>(static_cast<uint16_t>(val.at(i)));
            code                     = merge_surrogates(lead_surrogate, trail_surrogate);
            i++;
        }
        return true;
    }
};

template <>
struct unicode_reader<std::u32string>
{
    using string_type = std::u32string;
    using char_type   = typename string_type::value_type;

    const string_type& val;
    const bool         escape_unicode;

    unicode_reader(const string_type& val, const bool escape_unicode)
        : val(val)
        , escape_unicode(escape_unicode)
    {
    }

    bool get_code(size_t& i, uint32_t& code)
    {
        if (i >= val.size())
        {
            return false;
        }
        code = static_cast<uint32_t>(val.at(i));
        i++;
        return true;
    }
};

template <>
struct unicode_reader<std::u16string>
{
    using string_type = std::u16string;
    using char_type   = typename string_type::value_type;

    const string_type& val;
    const bool         escape_unicode;

    unicode_reader(const string_type& val, const bool escape_unicode)
        : val(val)
        , escape_unicode(escape_unicode)
    {
    }

    bool get_code(size_t& i, uint32_t& code)
    {
        if (i >= val.size())
        {
            return false;
        }

        code = static_cast<uint32_t>(static_cast<uint16_t>(val.at(i)));
        i++;

        if (!escape_unicode)
        {
            // code point will not be escaped
            return true;
        }

        if (JSONXX_UNICODE_SUR_LEAD_BEGIN <= code && code <= JSONXX_UNICODE_SUR_LEAD_END)
        {
            if (i >= val.size())
            {
                throw json_serialize_error("string was incomplete");
            }

            uint32_t lead_surrogate  = code;
            uint32_t trail_surrogate = static_cast<uint32_t>(static_cast<uint16_t>(val.at(i)));
            code                     = merge_surrogates(lead_surrogate, trail_surrogate);
            i++;
        }
        return true;
    }
};

//
// unicode_writer
//

template <typename _StrTy>
struct unicode_writer;

template <>
struct unicode_writer<std::string>
{
    using string_type   = std::string;
    using char_type     = typename string_type::value_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;

    string_type& buffer;

    unicode_writer(string_type& buffer)
        : buffer(buffer){};

    inline void add_char(const char_int_type ch)
    {
        buffer.push_back(char_traits::to_char_type(ch));
    }

    inline void add_code(uint32_t code)
    {
        if (!(0x00 <= code && code <= 0x10FFFF))
        {
            // invalid unicode
            return;
        }

        // Unicode              UTF-8
        // U+0000...U+007F      0xxxxxxx
        // U+0080...U+07FF      110xxxxx 10xxxxxx
        // U+0800...U+FFFF      1110xxxx 10xxxxxx 10xxxxxx
        // U+10000...U+10FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (code < 0x80)
        {
            // 0xxxxxxx
            add_char(static_cast<char_int_type>(code));
        }
        else if (code <= 0x7FF)
        {
            // 110xxxxx 10xxxxxx
            add_char(static_cast<char_int_type>(0xC0 | (code >> 6)));
            add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
        }
        else if (code <= 0xFFFF)
        {
            // 1110xxxx 10xxxxxx 10xxxxxx
            add_char(static_cast<char_int_type>(0xE0 | (code >> 12)));
            add_char(static_cast<char_int_type>(0x80 | ((code >> 6) & 0x3F)));
            add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
        }
        else
        {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            add_char(static_cast<char_int_type>(0xF0 | (code >> 18)));
            add_char(static_cast<char_int_type>(0x80 | ((code >> 12) & 0x3F)));
            add_char(static_cast<char_int_type>(0x80 | ((code >> 6) & 0x3F)));
            add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
        }
    }

    inline void add_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate)
    {
        add_code(merge_surrogates(lead_surrogate, trail_surrogate));
    }
};

template <>
struct unicode_writer<std::wstring>
{
    using string_type   = std::wstring;
    using char_type     = typename string_type::value_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;

    string_type& buffer;

    unicode_writer(string_type& buffer)
        : buffer(buffer){};

    inline void add_char(const char_int_type ch)
    {
        buffer.push_back(char_traits::to_char_type(ch));
    }

    inline void add_code(uint32_t code)
    {
        add_code(code, std::integral_constant<bool, sizeof(char_type) == 4>());
    }

    inline void add_code(uint32_t code, std::true_type /* sizeof(wchar_t) == 4 */)
    {
        add_char(static_cast<char_int_type>(code));
    }

    inline void add_code(uint32_t code, std::false_type /* sizeof(wchar_t) == 2 */)
    {
        if (code < 0xFFFF)
        {
            add_char(static_cast<char_int_type>(code));
        }
        else
        {
            throw json_parse_error("invalid 16-bits unicode character");
        }
    }

    inline void add_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate)
    {
        add_surrogates(lead_surrogate, trail_surrogate, std::integral_constant<bool, sizeof(char_type) == 4>());
    }

    inline void add_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate,
                               std::true_type /* sizeof(wchar_t) == 4 */)
    {
        add_code(merge_surrogates(lead_surrogate, trail_surrogate));
    }

    inline void add_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate,
                               std::false_type /* sizeof(wchar_t) == 2 */)
    {
        add_code(lead_surrogate);
        add_code(trail_surrogate);
    }
};

template <>
struct unicode_writer<std::u16string>
{
    using string_type   = std::u16string;
    using char_type     = typename string_type::value_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;

    string_type& buffer;

    unicode_writer(string_type& buffer)
        : buffer(buffer){};

    inline void add_char(const char_int_type ch)
    {
        buffer.push_back(char_traits::to_char_type(ch));
    }

    inline void add_code(uint32_t code)
    {
        if (code < 0xFFFF)
        {
            add_char(static_cast<char_int_type>(code));
        }
        else
        {
            throw json_parse_error("invalid 16-bits unicode character");
        }
    }

    inline void add_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate)
    {
        add_code(lead_surrogate);
        add_code(trail_surrogate);
    }
};

template <>
struct unicode_writer<std::u32string>
{
    using string_type   = std::u32string;
    using char_type     = typename string_type::value_type;
    using char_traits   = std::char_traits<char_type>;
    using char_int_type = typename char_traits::int_type;

    string_type& buffer;

    unicode_writer(string_type& buffer)
        : buffer(buffer){};

    inline void add_char(const char_int_type ch)
    {
        buffer.push_back(char_traits::to_char_type(ch));
    }

    inline void add_code(uint32_t code)
    {
        add_char(static_cast<char_int_type>(code));
    }

    inline void add_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate)
    {
        add_code(merge_surrogates(lead_surrogate, trail_surrogate));
    }
};

}  // namespace detail
}  // namespace jsonxx
