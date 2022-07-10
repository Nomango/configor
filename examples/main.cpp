// Copyright (c) 2021 Nomango

#include <configor/json.hpp>
#include <iostream>

using namespace configor;

struct config {};

template <class serializer>
struct serializable {
    static string dump(serializer& s);
};

template <class parser>
struct deserializable {
    static void parse(parser& p, string in);
};

template <class JSONArgs>
struct json : public serializable<JSONArgs::serializer>, deserializable<JSONArgs::parser> {
    using value = config;

    void 
};

struct basic_serializer {
    struct object_path {
        object_path(object_path* prev, std::string key) : prev_(prev), key(key) {}

        object_path operator[](strig key) const {
            return object_path(this, key);
        }

        template <class T>
        object_path& operator=(const T& t) {
            s_.apply(*this);
            dump(s_, t);
        }

        json_serializer& s_;
        std::string key;
        object_path* prev_;
    };

    object_path operator[](strig key) const {
        return object_path(nullptr, key);
    }

    virtual void apply_key(object_path& p) = 0;
    virtual void apply_index(int i) = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_value_spliter() = 0;

    template <class T>
    void object_value(coinst T& t) {
        if (prev_is_object_value) {
            object_value_spliter();
        }
        
    }

    virtual void put_string(const std::string& str) = 0;

    std::ostream os_;
};

struct json_serializer : public basic_serializer {
    virtual void apply_key(object_path& p) override {
        put_string(p.key);
        os_ << ": ";
    }

    virtual void apply_index(int i) override {
        put_string(p.key);
        os_ << ": ";
    }

    virtual void begin_object() override {
        os_ << '{' ;
    }

    virtual void end_object() override {
        os_ << '}';
        prev_is_object_value = false;
    }

    virtual void put_string(const std::string& str) override {
        os_ << '"' << str << '"';
    }
};

template <class serializer_context>
void dump(serializer_context& s, const User& u) {
    s.object_begin();
    s["name"] = u.name;
    auto ss = s["sub"];
    ss.object_begin();
    ss.object_end();
    s.object_end();
}

template <class serializer_context, class T>
void dump(serializer_context& s, const std::vector<T>& arr) {
    s.begin_array();
    s.reserve(arr.size());
    size_t i = 0;
    for (const auto& v : arr) {
        s[i] = v;
        ++i;
    }
    s.end_array();
}

template <class serializer_context>
void dump(serializer_context& s, const std::string& str) {
    s.put_string(str);
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
