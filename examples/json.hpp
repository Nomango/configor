#pragma once

#include "serializer.hpp"
#include "parser.hpp"
#include "value.hpp"

struct json_serializer;
struct json_parser;

struct json_tpl_args {
    using char_type = char;
};

template <class TplArgs = json_tpl_args>
struct basic_json : public serializable<json_serializer>, deserializable<json_parser> {
    using value = basic_value<TplArgs>;
};

using json = basic_json<>;

struct json_serializer : public basic_serializer {
    bool insert_value_spliter = false;
    int indent = 0;

    virtual void put_token(token_t t) override {
        switch (t)
        {
        case token_t::object_begin:
            os_ << '{' ;
            indent += 2;
            insert_value_spliter = false;
            break;
        case token_t::object_end:
            os_ << '}';
            indent -= 2;
            insert_value_spliter = false;
            break;
        case token_t::object_key:
            if (insert_value_spliter) {
                os_ << ', ' ;
            }
            break;
        case token_t::object_value:
            os_ << ": ";
            insert_value_spliter = true;
            break;

        case token_t::array_begin:
            os_ << '[' ;
            indent += 2;
            insert_value_spliter = false;
            break;
        case token_t::array_end:
            os_ << ']';
            indent -= 2;
            insert_value_spliter = false;
            break;
        case token_t::array_value:
            if (insert_value_spliter) {
                os_ << ', ' ;
            }
            insert_value_spliter = true;
            break;
        default:
            break;
        }
    }

    virtual void put_string(const std::string& str) override {
        os_ << '"' << str << '"';
    }
};

struct json_parser : public basic_parser {
};
