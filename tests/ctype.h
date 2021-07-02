// Copyright (c) 2021 Nomango

#if defined(__clang__)

#include <locale>

namespace std
{
template <>
class ctype<char16_t>
    : public std::locale::facet
    , public std::ctype_base
{
public:
    using char_type = char16_t;

    explicit ctype(size_t refs = 0)
        : locale::facet(refs)
    {
    }

    bool is(mask m, char_type c) const
    {
        return false;
    }

    const char_type* is(const char_type* low, const char_type* high, mask* vec) const
    {
        return high;
    }

    const char_type* scan_is(mask m, const char_type* low, const char_type* high) const
    {
        return high;
    }

    const char_type* scan_not(mask m, const char_type* low, const char_type* high) const
    {
        return high;
    }

    char_type toupper(char_type c) const
    {
        return c;
    }

    const char_type* toupper(char_type* low, const char_type* high) const
    {
        return high;
    }

    char_type tolower(char_type c) const
    {
        return c;
    }

    const char_type* tolower(char_type* low, const char_type* high) const
    {
        return high;
    }

    char_type widen(char c) const
    {
        return static_cast<char_type>(c);
    }

    const char* widen(const char* low, const char* high, char_type* to) const
    {
        return high;
    }

    char narrow(char_type c, char dfault) const
    {
        return static_cast<char>(c);
    }

    const char_type* narrow(const char_type* low, const char_type* high, char dfault, char* to) const
    {
        return high;
    }

    static locale::id id;

protected:
    ~ctype();
};

template <>
class ctype<char32_t>
    : public std::locale::facet
    , public std::ctype_base
{
public:
    using char_type = char32_t;

    explicit ctype(size_t refs = 0)
        : locale::facet(refs)
    {
    }

    bool is(mask m, char_type c) const
    {
        return false;
    }

    const char_type* is(const char_type* low, const char_type* high, mask* vec) const
    {
        return high;
    }

    const char_type* scan_is(mask m, const char_type* low, const char_type* high) const
    {
        return high;
    }

    const char_type* scan_not(mask m, const char_type* low, const char_type* high) const
    {
        return high;
    }

    char_type toupper(char_type c) const
    {
        return c;
    }

    const char_type* toupper(char_type* low, const char_type* high) const
    {
        return high;
    }

    char_type tolower(char_type c) const
    {
        return c;
    }

    const char_type* tolower(char_type* low, const char_type* high) const
    {
        return high;
    }

    char_type widen(char c) const
    {
        return static_cast<char_type>(c);
    }

    const char* widen(const char* low, const char* high, char_type* to) const
    {
        return high;
    }

    char narrow(char_type c, char dfault) const
    {
        return static_cast<char>(c);
    }

    const char_type* narrow(const char_type* low, const char_type* high, char dfault, char* to) const
    {
        return high;
    }

    static locale::id id;

protected:
    ~ctype();
};

}  // namespace std

#endif
