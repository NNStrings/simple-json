# simple-json

一个简单的基于 C++ 17 的 json 解析器

**接口：**

```cpp
auto [json, eaten] = simpjson::parse(str);
```

返回值为 `std::pair<JSONObject, size_t>`
