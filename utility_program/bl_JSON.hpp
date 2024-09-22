#ifndef _BOUNDLESS_JSON_HPP_FILE_
#define _BOUNDLESS_JSON_HPP_FILE_
#include <charconv>
#include <cstdint>
#include <cstring>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>
#include "bl_log.hpp"
namespace BL::JSON {
struct JSONObject;
using JSONDict = std::unordered_map<std::string, JSONObject>;
using JSONList = std::vector<JSONObject>;
struct JSONObject {
    using DataType = std::variant<std::monostate,  // null
                                  bool,            // boolean
                                  int64_t,         // integer
                                  double,          // floating point number
                                  std::string,     // string
                                  JSONList,        // list
                                  JSONDict         // dictionary
                                  >;
    DataType data;
};
std::optional<int64_t> tryParseNumInteger(const char* begin, const char* end) {
    int64_t val;
    int base, offset, neg = 1;
    if (*begin == '0') {
        if (*(begin + 1) == 'b' || *(begin + 1) == 'B')
            base = 2, offset = 2;
        else if (*(begin + 1) == 'x' || *(begin + 1) == 'X')
            base = 16, offset = 2;
        else
            base = 8, offset = 1;
    } else {
        base = 10;
        if (*begin == '+')
            offset = 1;
        else if (*begin == '-')
            offset = 1, neg = -1;
        else
            offset = 0;
    }
    auto res = std::from_chars(begin + offset, end, val, base);
    if (res.ec == std::errc() && res.ptr == end)
        return val * neg;
    return std::nullopt;
}
std::optional<double> tryParseNumFloat(const char* begin, const char* end) {
    double val;
    auto res = std::from_chars(begin, end, val);
    if (res.ec == std::errc() && res.ptr == end)
        return val;
    return std::nullopt;
}
char unescapedChar(char c) {
    switch (c) {
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case '0':
            return '\0';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case 'f':
            return '\f';
        case 'b':
            return '\b';
        case 'a':
            return '\a';
        default:
            return c;
    }
}
std::pair<JSONObject, size_t> parse(std::string_view json) {
    if (json.empty()) {
        print_error("JSON", "empty json string!");
        return {JSONObject{std::monostate{}}, 0u};
    }
    size_t i = 0;
    while (i < json.size() && std::isspace(json[i]))
        i++;
    json.remove_prefix(i);
    if (json[0] == 't' || json[0] == 'T' || json[0] == 'f' || json[0] == 'F') {
        char boolean_str[5]{};
        for (size_t j = 0; j < json.size() && j < 5; j++)
            boolean_str[j] = std::tolower(json[j]);
        bool boolean;
        if (std::strncmp(boolean_str, "false", 5) == 0)
            boolean = false, i += 5;
        else if (std::strncmp(boolean_str, "true", 4) == 0)
            boolean = true, i += 4;
        else {
            print_error("JSON", "Boolean string error!");
            goto PARSE_FAILED;
        }
        std::pair<JSONObject, size_t> result{
            JSONObject{JSONObject::DataType{std::in_place_index<1>, boolean}},
            i};
        return result;
    } else if (('0' <= json[0] && json[0] <= '9') || json[0] == '+' ||
               json[0] == '-') {
        static std::regex num_re{
            R"([+-]?([0][xXbB])?[0-9]+(\.[0-9]*)?([eE][+-]?[0-9]+)?)"};
        std::cmatch match;
        if (std::regex_search(json.begin(), json.end(), match, num_re)) {
            std::string str = match.str();
            if (auto num =
                    tryParseNumInteger(str.data(), str.data() + str.size());
                num.has_value()) {
                return {JSONObject{*num}, str.size() + i};
            }
            if (auto num =
                    tryParseNumFloat(str.data(), str.data() + str.size());
                num.has_value()) {
                return {JSONObject{*num}, str.size() + i};
            }
        }
        print_error("JSON", "Parse number error!");
    } else if (json[0] == '"' || json[0] == '\'') {
        char comma = json[0];
        std::string str;
        str.reserve(16);
        enum {
            Raw,
            Escaped,
        } phase = Raw;
        size_t j;
        for (j = 1; j < json.size(); j++) {
            char ch = json[j];
            if (phase == Raw) {
                if (ch == '\\') {
                    phase = Escaped;
                } else if (ch == comma) {
                    j += 1;
                    break;
                } else {
                    str.push_back(ch);
                }
            } else if (phase == Escaped) {
                str.push_back(unescapedChar(ch));
                phase = Raw;
            }
        }
        return {JSONObject{std::move(str)}, j + i};
    } else if (json[0] == '[') {
        JSONList res;
        size_t j;
        for (j = 1; j < json.size();) {
            while (std::isspace(json[j]))
                j++;
            if (json[j] == ']') {
                j++;
                break;
            }
            auto [obj, eaten] = parse(json.substr(j));
            if (eaten == 0) {
                print_error("JSON", "Parse list error!");
                j = 0, i = 0;
                break;
            }
            res.push_back(std::move(obj));
            j += eaten;
            while (std::isspace(json[j]))
                j++;
            if (json[j] == ',') {
                j++;
            } else if (json[j] == ']') {
                j++;
                break;
            } else {
                print_error("JSON", "List no devide comma!");
                j = 0, i = 0;
                break;
            }
        }
        return {JSONObject{std::move(res)}, j + i};
    } else if (json[0] == '{') {
        JSONDict res;
        size_t j;
        for (j = 1; j < json.size();) {
            while (std::isspace(json[j]))
                j++;
            if (json[j] == '}') {
                j++;
                break;
            }
            auto [keyobj, keyeaten] = parse(json.substr(j));
            if (keyeaten == 0) {
                print_error("JSON", "Parse dict key error!");
                j = 0, i = 0;
                break;
            }
            j += keyeaten;
            while (j < json.size() && isspace(json[j]))
                j++;
            if (json[j] == ':')
                j++;
            else {
                print_error("JSON", "Dict no devide colon!");
                j = 0, i = 0;
                break;
            }
            std::string* key = std::get_if<std::string>(&keyobj.data);
            if (key == nullptr) {
                print_error("JSON", "Parse dict key type error!");
                j = 0, i = 0;
                break;
            }
            auto [valobj, valeaten] = parse(json.substr(j));
            if (valeaten == 0) {
                print_error("JSON", "Parse dict value error!");
                j = 0, i = 0;
                break;
            }
            j += valeaten;
            res.try_emplace(std::move(*key), std::move(valobj));
            while (std::isspace(json[j]))
                j++;
            if (json[j] == ',') {
                j++;
            } else if (json[j] == '}') {
                j++;
                break;
            } else {
                print_error("JSON", "Dict no devide comma!");
                j = 0, i = 0;
                break;
            }
        }
        return {JSONObject{std::move(res)}, j + i};
    }
PARSE_FAILED:
    print_error("JSON", "Parse failed! ->", json, "<-");
    return {JSONObject{std::monostate{}}, 0u};
}
struct dump_visitor {
    std::stringstream& stm;
    void operator()(int64_t val) { stm << val; }
    void operator()(double val) { stm << val; }
    void operator()(bool val) { stm << (val ? "true" : "false"); }
    void operator()(const std::string& val) { stm << '\"' << val << '\"'; }
    void operator()(const JSONList& val) {
        stm << '[';
        size_t i = 0;
        for (i = 0; i + 1 < val.size(); i++) {
            std::visit(*this, val[i].data);
            stm << ',';
        }
        if (i < val.size())
            std::visit(*this, val[i].data);
        stm << ']';
    }
    void operator()(const JSONDict& val) {
        stm << '{';
        auto it = val.begin();
        while (true) {
            stm << '\"' << it->first << '\"' << ':';
            std::visit(*this, it->second.data);
            ++it;
            if (it != val.end())
                stm << ',';
            else
                break;
        }
        stm << '}';
    }
    void operator()(std::monostate v) { stm << "Error"; }
};
std::string dump(JSONObject& json) {
    std::stringstream stm;
    dump_visitor visitor{.stm = stm};
    std::visit(visitor, json.data);
    return stm.str();
}
}  // namespace BL::JSON
#endif  //!_BOUNDLESS_JSON_HPP_FILE_