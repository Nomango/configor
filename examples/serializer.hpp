#pragma once

#include <iostream>
#include <stack>
#include <vector>
#include <string>

#include "common.hpp"

template <class serializer>
struct serializable {
    template <class T>
    static std::string dump(const T& v) {
        serializer s{};
        dump(s, v);
    }

    template <class T>
    static std::string dump(serializer& s, const T& v) {
        serializer_context ctx{ s };
        ::dump(ctx, v);
    }
};

struct basic_serializer {
    std::ostream& os_;

    virtual void put_token(token_t) = 0;
    virtual void put_string(const std::string& str) = 0;
};

struct serializer_context {
    basic_serializer& s_;
    std::vector<std::string> path_;

    serializer_context(basic_serializer& s) : s_(s) {}

    serializer_context(const serializer_context& prev, std::string key) : s_(prev.s_), path_(prev.path_) {
        path_.push_back(key);
    }

    basic_serializer& get_serializer() const {
        return s_;
    }

    void put_token(token_t t) {
        s_.put_token(t);
    }

    struct object_context {
        serializer_context& parent_;
        object_context(serializer_context& parent, std::string key) : parent_(parent) {
            parent_.put_token(token_t::object_key);
            dump(parent_, key);
        }

        template <class T>
        object_context& operator=(const T& v) {
            parent_.put_token(token_t::object_value);
            dump(parent_, v);
        }
    };

    object_context operator[](std::string key) {
        return object_context{*this, key};
    }

    template <class T>
    void insert(std::string key, const T& v) {
        put_token(token_t::object_key);
        dump(*this, key);
        put_token(token_t::object_value);
        dump(*this, v);
    }

    template <class T>
    void push_back(const T& v) {
        put_token(token_t::array_value);
        dump(*this, v);
    }
};

template <class T>
void dump(serializer_context& ctx, const std::vector<T>& arr) {
    ctx.put_token(token_t::array_begin);
    for (const auto& v : arr) {
        ctx.push_back(v);
    }
    ctx.put_token(token_t::array_end);
}

void dump(serializer_context& ctx, const std::string& str) {
    ctx.get_serializer().put_string(str);
}
