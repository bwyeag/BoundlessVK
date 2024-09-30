#ifndef _BOUNDLESS_JSON_HPP_FILE_
#define _BOUNDLESS_JSON_HPP_FILE_
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
std::pair<JSONObject, size_t> parse(std::string_view json);
std::string dump(const JSONObject& json);
}  // namespace BL::JSON
#endif  //!_BOUNDLESS_JSON_HPP_FILE_