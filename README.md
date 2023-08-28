<div align="center">

![logo](./assets/logo.png)

*A light weight C++ JSON library*

![](https://github.com/muqiuhan/configor/actions/workflows/build.yml/badge.svg)
![](https://github.com/muqiuhan/configor/actions/workflows/test.yml/badge.svg)

</div>

## Features

- Header-only & STL-like
- Custom type conversion & serialization
- Complete Unicode support
- ASCII & Wide-character support

## Demo

### Create JSON objects:

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

### Conversion & Serialization

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


## LICENSE
MIT License

Copyright (c) 2019 Haibo

Copyright (c) 2023 MuqiuHan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.