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
    typedef char16_t char_type;

    explicit ctype(size_t refs = 0)
        : locale::facet(refs)
    {
    }

    bool is(mask m, char_type c) const
    {
        return do_is(m, c);
    }

    const char_type* is(const char_type* low, const char_type* high, mask* vec) const
    {
        return do_is(low, high, vec);
    }

    const char_type* scan_is(mask m, const char_type* low, const char_type* high) const
    {
        return do_scan_is(m, low, high);
    }

    const char_type* scan_not(mask m, const char_type* low, const char_type* high) const
    {
        return do_scan_not(m, low, high);
    }

    char_type toupper(char_type c) const
    {
        return do_toupper(c);
    }

    const char_type* toupper(char_type* low, const char_type* high) const
    {
        return do_toupper(low, high);
    }

    char_type tolower(char_type c) const
    {
        return do_tolower(c);
    }

    const char_type* tolower(char_type* low, const char_type* high) const
    {
        return do_tolower(low, high);
    }

    char_type widen(char c) const
    {
        return do_widen(c);
    }

    const char* widen(const char* low, const char* high, char_type* to) const
    {
        return do_widen(low, high, to);
    }

    char narrow(char_type c, char dfault) const
    {
        return do_narrow(c, dfault);
    }

    const char_type* narrow(const char_type* low, const char_type* high, char dfault, char* to) const
    {
        return do_narrow(low, high, dfault, to);
    }

    static locale::id id;

protected:
    ~ctype();
    virtual bool             do_is(mask m, char_type c) const;
    virtual const char_type* do_is(const char_type* low, const char_type* high, mask* vec) const;
    virtual const char_type* do_scan_is(mask m, const char_type* low, const char_type* high) const;
    virtual const char_type* do_scan_not(mask m, const char_type* low, const char_type* high) const;
    virtual char_type        do_toupper(char_type) const;
    virtual const char_type* do_toupper(char_type* low, const char_type* high) const;
    virtual char_type        do_tolower(char_type) const;
    virtual const char_type* do_tolower(char_type* low, const char_type* high) const;
    virtual char_type        do_widen(char) const;
    virtual const char*      do_widen(const char* low, const char* high, char_type* dest) const;
    virtual char             do_narrow(char_type, char dfault) const;
    virtual const char_type* do_narrow(const char_type* low, const char_type* high, char dfault, char* dest) const;
};

template <>
class ctype<char32_t>
    : public std::locale::facet
    , public std::ctype_base
{
public:
    typedef char32_t char_type;

    explicit ctype(size_t refs = 0)
        : locale::facet(refs)
    {
    }

    bool is(mask m, char_type c) const
    {
        return do_is(m, c);
    }

    const char_type* is(const char_type* low, const char_type* high, mask* vec) const
    {
        return do_is(low, high, vec);
    }

    const char_type* scan_is(mask m, const char_type* low, const char_type* high) const
    {
        return do_scan_is(m, low, high);
    }

    const char_type* scan_not(mask m, const char_type* low, const char_type* high) const
    {
        return do_scan_not(m, low, high);
    }

    char_type toupper(char_type c) const
    {
        return do_toupper(c);
    }

    const char_type* toupper(char_type* low, const char_type* high) const
    {
        return do_toupper(low, high);
    }

    char_type tolower(char_type c) const
    {
        return do_tolower(c);
    }

    const char_type* tolower(char_type* low, const char_type* high) const
    {
        return do_tolower(low, high);
    }

    char_type widen(char c) const
    {
        return do_widen(c);
    }

    const char* widen(const char* low, const char* high, char_type* to) const
    {
        return do_widen(low, high, to);
    }

    char narrow(char_type c, char dfault) const
    {
        return do_narrow(c, dfault);
    }

    const char_type* narrow(const char_type* low, const char_type* high, char dfault, char* to) const
    {
        return do_narrow(low, high, dfault, to);
    }

    static locale::id id;

protected:
    ~ctype();
    virtual bool             do_is(mask m, char_type c) const;
    virtual const char_type* do_is(const char_type* low, const char_type* high, mask* vec) const;
    virtual const char_type* do_scan_is(mask m, const char_type* low, const char_type* high) const;
    virtual const char_type* do_scan_not(mask m, const char_type* low, const char_type* high) const;
    virtual char_type        do_toupper(char_type) const;
    virtual const char_type* do_toupper(char_type* low, const char_type* high) const;
    virtual char_type        do_tolower(char_type) const;
    virtual const char_type* do_tolower(char_type* low, const char_type* high) const;
    virtual char_type        do_widen(char) const;
    virtual const char*      do_widen(const char* low, const char* high, char_type* dest) const;
    virtual char             do_narrow(char_type, char dfault) const;
    virtual const char_type* do_narrow(const char_type* low, const char_type* high, char dfault, char* dest) const;
};

}  // namespace std

#endif
