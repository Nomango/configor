# JSON-cpp

一个为 C++ 量身打造的轻量级 JSON 通用工具，轻松完成 JSON 解析和序列化功能，并和 C++ 输入输出流交互。

### 使用介绍

- 引入 JSON-cpp 头文件

```cpp
#include "nomango/json.hpp"
using namespace nomango;
```

- 使用 C++ 的方式的创建 JSON 对象

使用 `operator[]` 为 JSON 对象赋值

```cpp
json j;
j[L"number"] = 1;
j[L"float"] = 1.5;
j[L"string"] = L"this is a string";
j[L"boolean"] = true;
j[L"user"][L"id"] = 10;
j[L"user"][L"name"] = L"Nomango";
```

使用 `std::initializer_list` 为 JSON 对象赋值

```cpp
// 使用初始化列表构造数组
json arr = { 1, 2, 3 };
// 使用初始化列表构造对象
json obj = {
    {
        L"user", {
            { L"id", 10 },
            { L"name", L"Nomango" }
        }
    }
};
// 第二个对象
json obj2 = {
    { L"null", nullptr },
    { L"number", 1 },
    { L"float", 1.3 },
    { L"boolean", false },
    { L"string", L"中文测试" },
    { L"array", { 1, 2, true, 1.4 } },
    { L"object", { L"key", L"value" } }
};
```

使用辅助方法构造数组或对象

```cpp
json arr = json::array({ 1 });
json obj = json::object({ L"user", { { L"id", 1 }, { L"name", L"Nomango" } } });
```

- 判断 JSON 对象的值类型

```cpp
// 判断 JSON 值类型
bool is_null()
bool is_boolean()
bool is_integer()
bool is_float()
bool is_array()
bool is_object()
```

- 将 JSON 对象进行显式或隐式转换

```cpp
// 显示转换
auto b = j[L"boolean"].as_boolean();        // bool
auto i = j[L"number"].as_integer();         // int32_t
auto f = j[L"float"].as_float();            // float
const auto& arr = j[L"array"].as_array();   // arr 实际是 std::vector<json> 类型
const auto& obj = j[L"user"].as_object();   // obj 实际是 std::map<std::wstring, json> 类型
```

```cpp
// 隐式转换
bool b = j[L"boolean"];
int i = j[L"number"];           // int32_t 自动转换为 int
double d = j[L"float"];         // float 自动转换成 double
std::vector<json> arr = j[L"array"];
std::map<std::wstring, json> obj = j[L"user"];
```

> 若 JSON 值类型与待转换类型不相同也不协变，会引发 json_type_error 异常

- 取值的同时判断类型

```cpp
int n;
bool ret = j[L"boolean"].get_value(&n); // 若取值成功，ret 为 true
```

- JSON 对象类型和数组类型的遍历

```cpp
// 增强 for 循环
for (auto& j : obj) {
    std::wcout << j << std::endl;
}
```

```cpp
// 使用迭代器遍历
for (auto iter = obj.begin(); iter != obj.end(); iter++) {
    std::wcout << iter.key() << L":" << iter.value() << std::endl;
}
```

- JSON 解析

```cpp
// 解析字符串
json j = json::parse(L"{ \"happy\": true, \"pi\": 3.141 }");
```

```cpp
// 从文件读取 JSON
std::wifstream ifs(L"sample.json");

json j;
ifs >> j;
```

```cpp
// 从标准输入流读取 JSON
json j;
std::wcin >> j;
```

- JSON 序列化

```cpp
// 序列化为字符串
std::wstring json_str = j.dump();
// 美化输出，使用 4 个空格对输出进行格式化
std::wstring pretty_str = j.dump(4, ' ');
```

```cpp
// 将 JSON 内容输出到文件
std::wofstream ofs(L"output.json");
ofs << j << std::endl;
```

```cpp
// 将 JSON 内容输出到文件，并美化
std::wofstream ofs(L"pretty.json");
ofs << std::setw(4) << j << std::endl;
```

```cpp
// 将 JSON 内容输出到标准输出流
json j;
std::wcout << j;    // 可以使用 std::setw(4) 对输出内容美化
```

### 更多

若你需要将 JSON 解析和序列化应用到非 std::basic_stream 流中，可以通过创建自定义 `output_adapter` 和 `input_adapter` 的方式实现。

实际上 json::parse() 和 json::dump() 函数也是通过自定义的 `string_output_adapter` 和 `string_input_adapter` 实现对字符串内容的输入和输出。

详细内容请参考 json_parser.hpp 和 json_serializer.hpp

### 鸣谢

感谢 [nlohmann](https://github.com/nlohmann/json) 的 `JSON for Modern C++` 项目，本仓库的许多概念和灵感都来源于此。
