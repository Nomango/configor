<div align="center">

![logo](./assets/logo.png)

<!-- [![Open in VSCode](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/Nomango/configor) -->
[![Open in VSCode](https://img.shields.io/badge/open-in%20Visual%20Studio%20Code-blue)](https://open.vscode.dev/Nomango/configor)
[![Github status](https://github.com/Nomango/configor/actions/workflows/unit_tests.yml/badge.svg?branch=master)](https://github.com/Nomango/configor/actions)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/cf98f6b174fe4dd19f1e4574ac527a07)](https://www.codacy.com/gh/Nomango/configor/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Nomango/configor&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/Nomango/configor/branch/master/graph/badge.svg?token=OO71U89I5N)](https://codecov.io/gh/Nomango/configor)
[![GitHub release](https://img.shields.io/github/release/nomango/configor)](https://github.com/Nomango/configor/releases/latest)
[![GitHub license](https://img.shields.io/github/license/nomango/configor)](https://github.com/Nomango/configor/blob/master/LICENSE)

A light weight configuration library for C++11.

[EN](./README.md) | [中文](./README-zh.md)

</div>

## Features

- Header-only & STL-like
- Custom type conversion & serialization
- Complete Unicode support
- ASCII & Wide-character support

## Quick start

Create JSON objects:

```cpp
json::value j;
j["integer"] = 1;
j["float"] = 1.5;
j["string"] = "something";
j["boolean"] = true;
j["user"]["id"] = 10;
j["user"]["name"] = "Nomango";

json::value j2 = json::object{
    { "null", nullptr },
    { "integer", 1 },
    { "float", 1.3 },
    { "boolean", true },
    { "string", "something" },
    { "array", json::array{ 1, 2 } },
    { "object", json::object{
        { "key", "value" },
        { "key2", "value2" },
    }},
};
```

Conversion & Serialization:

```cpp
struct User
{
    std::string name;
    int age;

    // bind custom type to configor
    CONFIGOR_BIND(json::value, User, REQUIRED(name), OPTIONAL(age))
};

// User -> json
json::value j = User{"John", 18};
// json -> User
User u = json::object{{"name", "John"}, {"age", 18}};

// User -> string
std::string str = json::dump(User{"John", 18});
// string -> User
User u = json::parse("{\"name\": \"John\", \"age\": 18}");

// User -> stream
std::cout << json::wrap(User{"John", 18});
// stream -> User
User u;
std::cin >> json::wrap(u);
```

Learn more from the [wiki](https://github.com/Nomango/configor/wiki).

## Plan

- [x] Custom type conversion
- [x] Unicode support
- [x] Unit test coverage above 85%
- [ ] Improve error message
- [ ] YAML support
- [ ] ini support
- [ ] json5 support
- [ ] SAX tool
