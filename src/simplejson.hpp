#ifndef SIMP_JSON_HPP
#define SIMP_JSON_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <variant>
#include <cstring>
#include <optional>
#include <charconv>
#include <regex>

namespace simpjson {
struct JSONObject;

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

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "[ ";
    for (size_t i = 0; i < v.size(); i++) {
        os << v.at(i);
        if (i != v.size() - 1) {
            os << ", ";
        }
    }
    os << " ]";
    return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<T1, T2>& v) {
    auto print_key_value = [&] (const auto& key, const auto& value) {
        os << key << ": " << value;
    };
    os << "{ ";
    size_t i = 0;
    for (const auto& [key, value] : v) {
        print_key_value(key, value);
        if (++i != v.size()) {
            os << ", ";
        }
    }
    os << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const JSONObject& json) {
    std::visit([&] (const auto& t) {
        if constexpr (std::is_same_v<std::decay_t<decltype(t)>, bool>) {
            os << (t ? "true" : "false");
        }
        else {
            os << t;
        }
    }, json.inner);
    return os;
}

template<typename T>
std::optional<T> try_parse_num(std::string_view str) {
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

char unescaped_char(char c) {
    switch (c) {
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    default: return c;
    }
}

std::pair<JSONObject, size_t> parse(std::string_view json) {
    if (json.empty()) {
        return {JSONObject{std::nullptr_t{}}, 0};
    }
    else if (size_t off = json.find_first_not_of(" \n\v\t\r\f\0"); off && off != json.npos) {
        auto [obj, eaten] = parse(json.substr(off));
        return {std::move(obj), eaten + off};
    }
    else if (json.substr(0, 4) == "true") {
        return {JSONObject{true}, 4};
    }
    else if (json.substr(0, 5) == "false") {
        return {JSONObject{false}, 5};
    }
    else if (json[0] >= '0' && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if (std::regex_search(json.begin(), json.end(), match, num_re)) {
            std::string str = match.str();
            if (auto num = try_parse_num<int>(str)) {
                return {JSONObject{*num}, str.size()};
            }
            if (auto num = try_parse_num<double>(str)) {
                return {JSONObject{*num}, str.size()};
            }
        }
    }
    else if (json[0] == '"') {
        std::string str;
        enum {RAW, ESACPED} phase = RAW;
        size_t i;
        for (i = 1; json.size(); i++) {
            char ch = json[i];
            if (phase == RAW) {
                if (ch == '\\') {
                    phase = ESACPED;
                }
                else if (ch == '"') {
                    ++i;
                    break;
                }
                else {
                    str += ch;
                }
            }
            else {
                phase = RAW;
                str += unescaped_char(json[i]);
            }
        }
        return {JSONObject{std::move(str)}, i};
    }
    else if (json[0] == '[') {
        std::vector<JSONObject> res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == ']') {
                ++i;
                break;
            }
            auto [obj, eaten] = parse(json.substr(i));
            if (!eaten) {
                i = 0;
                break;
            }
            res.push_back(std::move(obj));
            i += eaten;
            if (json[i] == ',') {
                ++i;
            }
        }
        return {JSONObject{std::move(res)}, i};
    }
    else if (json[0] == '{') {
        std::unordered_map<std::string, JSONObject> res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == '}') {
                ++i;
                break;
            }
            auto [keyobj, keyeaten] = parse(json.substr(i));
            if (!keyeaten || !std::holds_alternative<std::string>(keyobj.inner)) {
                i = 0;
                break;
            }
            i += keyeaten;
            if (json[i] == ':') {
                ++i;
            }
            std::string key = std::move(std::get<std::string>(keyobj.inner));
            auto [valobj, valeaten] = parse(json.substr(i));
            if (!valeaten) {
                i = 0;
                break;
            }
            i += valeaten;
            res.insert_or_assign(std::move(key), std::move(valobj));
            if (json[i] == ',') {
                ++i;
            }
        }
        return {JSONObject{std::move(res)}, i};
    }
    return {JSONObject{std::nullptr_t{}}, 0};
}

[[nodiscard]] std::string stringify(const JSONObject& json) {
    std::string json_string;
    std::stringstream ss;
    ss << json;
    std::string tmp;
    while (ss >> tmp) {
        json_string += tmp + " ";
    }
    return json_string;
}

}

#endif
