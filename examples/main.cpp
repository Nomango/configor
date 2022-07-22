// Copyright (c) 2021 Nomango

#include "json.hpp"

struct User{
    int id;
    std::string name;
};

void dump(serializer_context& ctx, const User& u) {
    ctx.put_token(token_t::object_begin);
    ctx["name"] = u.name;
    ctx.insert("name", u.name);
    ctx.put_token(token_t::object_end);
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
    return 0;
}
