#pragma once

#include <string>

#include "common.hpp"

template <class parser>
struct deserializable {
    template <class T>
    static void parse(const std::string& in, T& v) {
        parser p{};
        parse(p, in, v);
    }

    template <class T>
    static void parse(parser& p, const std::string& in, T& v) {
        parser_context ctx { p };
        ::parse(ctx, v);
    }
};

struct basic_parser {
    std::istream& is_;

    virtual token_t get_token() = 0;
    virtual std::string get_string() = 0;
};

struct parser_context {
    basic_parser& p_;

    parser_context(basic_parser& p) : p_(p) {}

    void must(token_t t) {
        if (t != p_.get_token()) {
            throw;
        }
    }

    basic_parser& get_parser() const {
        return p_;
    }
};

void parse(parser_context& ctx, std::string& str) {
    ctx.must(token_t::value_string);
    str = ctx.get_parser().get_string();
}
