// Copyright (c) 2021 Nomango

// #include <configor/json.hpp>
#include <iostream>
#include <stack>
#include <vector>

// using namespace configor;

struct User{
    int id;
    std::string name;
};

struct value {};

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

template <class parser>
struct deserializable {
    static void parse(parser& p, std::string in);
};

struct jsonArgs {
    using serializer = json_serializer;
    using parser = void;
};

template <class JSONArgs = jsonArgs>
struct basic_json : public serializable<JSONArgs::serializer>, deserializable<JSONArgs::parser> {
    using value = value;
};

using json = basic_json<>;

enum class token_t {
    object_begin,
    object_end,
    object_key,
    object_value,

    array_begin,
    array_end,
    array_value,
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

struct json_serializer : public basic_serializer {
    bool insert_value_spliter = false;

    virtual void put_token(token_t t) override {
        switch (t)
        {
        case token_t::object_begin:
            os_ << '{' ;
            insert_value_spliter = false;
            break;
        case token_t::object_end:
            os_ << '}';
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
            insert_value_spliter = false;
            break;
        case token_t::array_end:
            os_ << ']';
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

void dump(serializer_context& ctx, const User& u) {
    ctx.put_token(token_t::object_begin);
    ctx["name"] = u.name;
    ctx.insert("name", u.name);
    ctx.put_token(token_t::object_end);
}

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

int main(int argc, char** argv)
{
    std::string str;

    User u;
    json::parse(str, u);
    str = json::dump(u);

    json::value v;
    json::parse(str, v);
    str = json::dump(v);

    u = User(v);
    v = json::value(u);

    // json::value v{ "sdf" };
    // std::cout << "v is string " << (v.is_string()) << std::endl;

    // {
    //     json::value k{ v };
    //     std::cout << "k is string " << (k.is_string()) << std::endl;
    //     std::cout << "v is string " << (v.is_string()) << std::endl;
    // }

    // {
    //     json::value j{ std::move(v) };
    //     std::cout << "j is string " << (j.is_string()) << std::endl;
    //     std::cout << "v is string " << (v.is_string()) << std::endl;
    // }

    // {
    //     wjson::value w{ L"sdf" };
    //     std::cout << "w is string " << (w.is_string()) << std::endl;
    // }
    return 0;
}
