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
#include <array>        // std::array
#include <cstdint>      // uint32_t, uint8_t
#include <iomanip>      // std::fill, std::setw, std::setprecision, std::right, std::noshowbase
#include <istream>      // std::basic_istream
#include <ostream>      // std::basic_ostream
#include <type_traits>  // std::char_traits

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

namespace
{

inline uint32_t merge_surrogates(uint32_t lead_surrogate, uint32_t trail_surrogate)
{
    uint32_t codepoint = ((lead_surrogate - JSONXX_UNICODE_SUR_LEAD_BEGIN) << JSONXX_UNICODE_SUR_BITS);
    codepoint += (trail_surrogate - JSONXX_UNICODE_SUR_TRAIL_BEGIN);
    codepoint += JSONXX_UNICODE_SUR_BASE;
    return codepoint;
}

inline void separate_surrogates(uint32_t codepoint, uint32_t& lead_surrogate, uint32_t& trail_surrogate)
{
    codepoint       = codepoint - JSONXX_UNICODE_SUR_BASE;
    lead_surrogate  = static_cast<uint16_t>(JSONXX_UNICODE_SUR_LEAD_BEGIN + (codepoint >> JSONXX_UNICODE_SUR_BITS));
    trail_surrogate = static_cast<uint16_t>(JSONXX_UNICODE_SUR_TRAIL_BEGIN + (codepoint & JSONXX_UNICODE_SUR_MAX));
}

}  // namespace

}  // namespace detail

template <typename _CharTy>
class utf8
{
public:
    using char_type    = _CharTy;
    using char_traits  = std::char_traits<char_type>;
    using istream_type = std::basic_istream<char_type>;
    using ostream_type = std::basic_ostream<char_type>;

    static void encode(ostream_type& os, uint32_t codepoint)
    {
        // Unicode              UTF-8
        // U+0000...U+007F      0xxxxxxx
        // U+0080...U+07FF      110xxxxx 10xxxxxx
        // U+0800...U+FFFF      1110xxxx 10xxxxxx 10xxxxxx
        // U+10000...U+10FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (codepoint < 0x80)
        {
            // 0xxxxxxx
            os << static_cast<char_type>(codepoint);
        }
        else if (codepoint <= 0x7FF)
        {
            // 110xxxxx 10xxxxxx
            os << static_cast<char_type>(0xC0 | (codepoint >> 6));
            os << static_cast<char_type>(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint <= 0xFFFF)
        {
            // 1110xxxx 10xxxxxx 10xxxxxx
            os << static_cast<char_type>(0xE0 | (codepoint >> 12));
            os << static_cast<char_type>(0x80 | ((codepoint >> 6) & 0x3F));
            os << static_cast<char_type>(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint <= 0x10FFFF)
        {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            os << static_cast<char_type>(0xF0 | (codepoint >> 18));
            os << static_cast<char_type>(0x80 | ((codepoint >> 12) & 0x3F));
            os << static_cast<char_type>(0x80 | ((codepoint >> 6) & 0x3F));
            os << static_cast<char_type>(0x80 | (codepoint & 0x3F));
        }
        else
        {
            os.setstate(std::ios_base::failbit);
        }
    }

    static bool decode(istream_type& is, uint32_t& codepoint)
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

        // peek one byte and check eof
        const auto first_byte = static_cast<uint8_t>(is.peek());

        if (is.eof())
            return false;

        codepoint = 0;

        // read bytes
        const auto extra_bytes_to_read = utf8_extra_bytes[first_byte];
        switch (extra_bytes_to_read)
        {
        case 5:
            codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
            codepoint <<= 6;
        case 4:
            codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
            codepoint <<= 6;
        case 3:
            codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
            codepoint <<= 6;
        case 2:
            codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
            codepoint <<= 6;
        case 1:
            codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
            codepoint <<= 6;
        case 0:
            codepoint += static_cast<uint32_t>(static_cast<uint8_t>(is.get()));
        }
        codepoint -= utf8_offsets[extra_bytes_to_read];
        return true;
    }
};

template <typename _CharTy>
class utf16
{
public:
    using char_type    = _CharTy;
    using char_traits  = std::char_traits<char_type>;
    using istream_type = std::basic_istream<char_type>;
    using ostream_type = std::basic_ostream<char_type>;

    static_assert(sizeof(char_type) >= 2, "The size of utf16 characters must be larger than 16 bits");

    static inline void encode(ostream_type& os, uint32_t codepoint)
    {
        if (codepoint <= 0xFFFF)
        {
            os << char_traits::to_char_type(static_cast<typename char_traits::int_type>(codepoint));
        }
        else if (codepoint <= 0x10FFFF)
        {
            uint32_t lead_surrogate = 0, trail_surrogate = 0;
            detail::separate_surrogates(codepoint, lead_surrogate, trail_surrogate);
            os << char_traits::to_char_type(static_cast<typename char_traits::int_type>(lead_surrogate));
            os << char_traits::to_char_type(static_cast<typename char_traits::int_type>(trail_surrogate));
        }
        else
        {
            os.setstate(std::ios_base::failbit);
        }
    }

    static inline bool decode(istream_type& is, uint32_t& codepoint)
    {
        codepoint = static_cast<uint32_t>(static_cast<uint16_t>(is.get()));

        if (is.eof())
            return false;

        if (JSONXX_UNICODE_SUR_LEAD_BEGIN <= codepoint && codepoint <= JSONXX_UNICODE_SUR_LEAD_END)
        {
            uint32_t lead_surrogate  = codepoint;
            uint32_t trail_surrogate = static_cast<uint32_t>(static_cast<uint16_t>(is.get()));

            if (JSONXX_UNICODE_SUR_TRAIL_BEGIN <= trail_surrogate && trail_surrogate <= JSONXX_UNICODE_SUR_TRAIL_END)
            {
                codepoint = detail::merge_surrogates(lead_surrogate, trail_surrogate);
            }
            else
            {
                is.setstate(std::ios_base::failbit);
            }
        }

        if (codepoint > 0x10FFFF)
        {
            is.setstate(std::ios_base::failbit);
        }
        return true;
    }
};

template <typename _CharTy>
class utf32
{
public:
    using char_type    = _CharTy;
    using char_traits  = std::char_traits<char_type>;
    using istream_type = std::basic_istream<char_type>;
    using ostream_type = std::basic_ostream<char_type>;

    static_assert(sizeof(char_type) >= 4, "The size of utf32 characters must be larger than 32 bits");

    static inline void encode(ostream_type& os, uint32_t codepoint)
    {
        if (codepoint > 0x10FFFF)
        {
            os.setstate(std::ios_base::failbit);
        }
        os << char_traits::to_char_type(static_cast<typename char_traits::int_type>(codepoint));
    }

    static inline bool decode(istream_type& is, uint32_t& codepoint)
    {
        codepoint = static_cast<uint32_t>(is.get());

        if (is.eof())
            return false;

        if (codepoint > 0x10FFFF)
        {
            is.setstate(std::ios_base::failbit);
        }
        return true;
    }
};

template <typename _CharTy>
class auto_utf
{
public:
    using char_type    = _CharTy;
    using char_traits  = std::char_traits<char_type>;
    using istream_type = std::basic_istream<char_type>;
    using ostream_type = std::basic_ostream<char_type>;

    static inline void encode(ostream_type& os, uint32_t codepoint)
    {
        encode(os, codepoint, std::integral_constant<int, sizeof(_CharTy)>());
    }

    static inline bool decode(istream_type& is, uint32_t& codepoint)
    {
        return decode(is, codepoint, std::integral_constant<int, sizeof(_CharTy)>());
    }

private:
    static inline void encode(ostream_type& os, uint32_t codepoint, std::integral_constant<int, 1>)
    {
        utf8<_CharTy>::encode(os, codepoint);
    }

    static inline void encode(ostream_type& os, uint32_t codepoint, std::integral_constant<int, 2>)
    {
        utf16<_CharTy>::encode(os, codepoint);
    }

    static inline void encode(ostream_type& os, uint32_t codepoint, std::integral_constant<int, 4>)
    {
        utf32<_CharTy>::encode(os, codepoint);
    }

    static inline bool decode(istream_type& is, uint32_t& codepoint, std::integral_constant<int, 1>)
    {
        return utf8<_CharTy>::decode(is, codepoint);
    }

    static inline bool decode(istream_type& is, uint32_t& codepoint, std::integral_constant<int, 2>)
    {
        return utf16<_CharTy>::decode(is, codepoint);
    }

    static inline bool decode(istream_type& is, uint32_t& codepoint, std::integral_constant<int, 4>)
    {
        return utf32<_CharTy>::decode(is, codepoint);
    }
};

}  // namespace jsonxx
