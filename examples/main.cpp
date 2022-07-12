// Copyright (c) 2021 Nomango

#include <configor/json.hpp>
#include <iostream>

using namespace configor;

struct config {};

template <class serializer>
struct serializable {
    template <class T>
    static string dump(serializer& s, const T& v) {
        serializer_context ctx{ s };
        ::dump(s, v);
    }
};

template <class parser>
struct deserializable {
    static void parse(parser& p, string in);
};

template <class JSONArgs>
struct json : public serializable<JSONArgs::serializer>, deserializable<JSONArgs::parser> {
    using value = config;
};

enum class token_t {
    object_begin,
    object_end,
    object_value_spliter,

    array_begin,
    array_end,
    array_value_spliter,
};

struct basic_serializer {
    std::ostream& os_;
    bool prev_is_object_value_;

    virtual void apply_key(std::vector<std::string> path) = 0;
    virtual void apply_index(int i) = 0;
    virtual void apply_token(token_t) = 0;

    template <class T>
    void object_value(const T& t) {
        if (prev_is_object_value_) {
            object_value_spliter();
        }
        dump()
    }

    virtual void put_string(const std::string& str) = 0;
};

struct serializer_context {
    basic_serializer& s_;
    std::vector<std::string> path_;

    serializer_context(basic_serializer& s) : s_(s) {}

    serializer_context(const serializer_context& prev, std::string key) : s_(prev.s_), path_(prev.path_) {
        path_.push_back(key);
    }

    serializer_context operator[](std::string key) const {
        return serializer_context(*this, key);
    }

    template <class T>
    serializer_context& operator=(const T& t) {
        s_.apply_key(path_);
        dump(s_, t);
    }

    basic_serializer& get_serializer() const {
        return s_;
    }
};

struct json_serializer : public basic_serializer {
    virtual void apply_key(std::vector<std::string> path) override {
        put_string(p.key);
        os_ << ": ";
    }

    virtual void apply_index(int i) override {
        put_string(p.key);
        os_ << ": ";
    }

    virtual void apply_token(token_t t) override {
        switch (t)
        {
        case token_t::object_begin:
            os_ << '{' ;
            break;
        case token_t::object_end:
            os_ << '}';
            prev_is_object_value_ = false;
            break;
        case token_t::object_value_spliter:
            os_ << ', ' ;
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
    auto& s = ctx.get_serializer();
    s.apply_token(token_t::object_begin);
    ctx["name"] = u.name;
    s.apply_token(token_t::object_end);
}

template <class T>
void dump(serializer_context& ctx, const std::vector<T>& arr) {
    auto& s = ctx.get_serializer();
    s.apply_token(token_t::array_begin);
    s.reserve(arr.size());
    size_t i = 0;
    for (const auto& v : arr) {
        ctx[i] = v;
        ++i;
    }
    s.apply_token(token_t::array_end);
}

void dump(serializer_context& ctx, const std::string& str) {
    ctx.get_serializer().put_string(str);
}

int main(int argc, char** argv)
{
    User u;
    json::parse(str, u);
    str = json::dump(u);

    json::value v;
    json::parse(str, v);
    str = json::dump(v);

    u = User(v);
    v = json::value(u);

    json::value v{ "sdf" };
    std::cout << "v is string " << (v.is_string()) << std::endl;

    {
        json::value k{ v };
        std::cout << "k is string " << (k.is_string()) << std::endl;
        std::cout << "v is string " << (v.is_string()) << std::endl;
    }

    {
        json::value j{ std::move(v) };
        std::cout << "j is string " << (j.is_string()) << std::endl;
        std::cout << "v is string " << (v.is_string()) << std::endl;
    }

    {
        wjson::value w{ L"sdf" };
        std::cout << "w is string " << (w.is_string()) << std::endl;
    }
    return 0;
}
