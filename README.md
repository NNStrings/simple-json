# simple-json

一个简单的基于 C++ 17 的 json 解析器

**json数据结构：**

```cpp
using JSONList = std::vector<JSONObject>;
using JSONDict = std::unordered_map<std::string, JSONObject>;

struct JSONObject {
    std::variant
    <std::nullptr_t,
     bool,
     int,
     double,
     std::string,
     JSONList,
     JSONDict
    > inner;

    template <typename T>
    bool is() const {
        return std::holds_alternative<T>(inner);
    }

    template <typename T>
    T& get() const {
        return std::get<T>(inner);
    }

    template <typename T>
    T& get() {
        return std::get<T>(inner);
    }
};
```

**接口：**

将字符串解析成 `json` 格式

```cpp
std::pair<JSONObject, size_t> parse(std::string_view json);
```

将 `json` 格式解析成字符串（实际这里偷懒了，复用了 `<<` 重载，没有效率）

```cpp
std::string stringify(const JSONObject& json);
```