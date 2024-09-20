#include <iostream>
#include <string>
#include "simplejson.hpp"

int main()
{
    std::string_view str = R"(
    {
        "a": 123.23,
        "hello": ["c", "cpp", "py"],
        "ktt": true,
        "ymm": false,
        "xmm":
        {
            "v": "vvv"
        }
    }
    )";

    auto [json, eaten] = simpjson::parse(str);
    std::string ans = simpjson::stringify(json);

    std::cout << json << std::endl;
    const auto& dict = json.get<simpjson::JSONDict>().at("hello");
    auto dovisit = [&] (auto& dovisit, const simpjson::JSONObject& content) -> void {
        std::visit([&] (const auto& content) {
            if constexpr (std::is_same_v<std::decay_t<decltype(content)>, simpjson::JSONList>) {
                for (const auto& iter : content) {
                    dovisit(dovisit, iter);
                }
            }
            else {
                std::cout << "visit: " << content << std::endl;
            }
        }, content.inner);
    };
    dovisit(dovisit, dict);
    return 0;
}