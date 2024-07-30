#include "bl_helpers_funct.hpp"
#include "bl_file_def.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace bl;
const char str_menu[] =
    "0.q/quit:quit\n1.h/help: print menu\n2.v/vertex: vertex class "
    "code\n3.ep/expression: mathexpression compute\n4.s/shader: create shader file\n"
    "5.m/model: create model file\n";
enum Order { Quit, Menu, VertexGen, ExpressionCompute, ShaderCreate , ModelCreate};
const std::map<std::string, Order> get_order{{"q", Quit},
                                             {"quit", Quit},
                                             {"h", Menu},
                                             {"help", Menu},
                                             {"v", VertexGen},
                                             {"vertex", VertexGen},
                                             {"ep", ExpressionCompute},
                                             {"expression", ExpressionCompute},
                                             {"s", ShaderCreate},
                                             {"shader", ShaderCreate},
                                             {"m",ModelCreate},
                                             {"model",ModelCreate}};
void generateVertexCode();
void expressionProcess();
void shaderCreate();
void modelCreate();
void createShaderFile(const std::string& path,const std::vector<std::string>& data,const std::vector<vk::ShaderStageFlagBits>& stages);
int main() {
    std::cout << "Code Generater\n" << str_menu;
    std::string order;
    while (true) {
        std::cout << ">>";
        std::getline(std::cin, order);
        auto it = get_order.find(order);
        if (it != get_order.end()) {
            if (it->second == Quit) {
                std::cout << "Quit.\n" << std::endl;
                return 0;
            } else if (it->second == Menu) {
                std::cout << str_menu;
            } else if (it->second == VertexGen) {

                generateVertexCode();
            } else if (it->second == ExpressionCompute) {
                expressionProcess();
            } else if (it->second == ShaderCreate) {
                try {
                    shaderCreate();
                }
                catch(const std::exception& e) {
                    std::cerr << "exception rised in creating:" << e.what() << '\n';
                }
            } else if (it->second == ModelCreate) {
                try {
                    modelCreate();
                }
                catch(const std::exception& e) {
                    std::cerr << "exception rised in creating:" << e.what() << '\n';
                }
            }
        } else {
            std::cout << "invalid order:" << order << ";\n";
        }
        std::cin.ignore();
    }
}
struct VertexType {
    std::string& format;
    uint32_t loc, offset, stride;
};
struct VertexData {
    std::string format;
    uint32_t size;
    std::string type;
    uint32_t count;
};
void generateVertexCode() {
    std::cout << "Generate Vertex Code:\n";
    static const char type_list[] =
        "float\tvec2~4\ndouble\tdvec2~4\nint\tivec2~4\nuint\tuvec2~"
        "4\nbool\tbvec2~4\n#matN/dmatN\tM,N in {2,3,4}\n";
    static std::map<std::string, VertexData> get_format{
        {"float", VertexData{"vk::Format::eR32Sfloat", 4, "float", 1}},
        {"vec2", VertexData{"vk::Format::eR32G32Sfloat", 8, "float", 2}},
        {"vec3", VertexData{"vk::Format::eR32G32B32Sfloat", 12, "float", 3}},
        {"vec4", VertexData{"vk::Format::eR32G32B32A32Sfloat", 16, "float", 4}},
        {"double", VertexData{"vk::Format::eR32Sfloat", 8, "double", 1}},
        {"dvec2", VertexData{"vk::Format::eR64G64Sfloat", 16, "double", 2}},
        {"dvec3", VertexData{"vk::Format::eR64G64B64Sfloat", 24, "double", 3}},
        {"dvec4",
         VertexData{"vk::Format::eR64G64B64A64Sfloat", 32, "double", 4}},
        {"int", VertexData{"vk::Format::eR32Sint", 4, "int32_t", 1}},
        {"ivec2", VertexData{"vk::Format::eR64G64Sint", 8, "int32_t", 2}},
        {"ivec3", VertexData{"vk::Format::eR64G64B64Sint", 12, "int32_t", 3}},
        {"ivec4",
         VertexData{"vk::Format::eR64G64B64A64Sint", 16, "int32_t", 4}},
        {"uint", VertexData{"vk::Format::eR32Uint", 4, "uint32_t", 1}},
        {"uvec2", VertexData{"vk::Format::eR64G64Uint", 8, "uint32_t", 2}},
        {"uvec3", VertexData{"vk::Format::eR64G64B64Uint", 12, "uint32_t", 3}},
        {"uvec4",
         VertexData{"vk::Format::eR64G64B64A64Uint", 16, "uint32_t", 4}}};
    static const char* help =
        "Input Format:location,type;q to quit;h to get help;\nc to cancel;\n";
    std::cout << type_list << help;
    uint32_t offset = 0;
    std::vector<VertexType> vtarr;
    std::stringstream sstream;
    int cnt = 0;
    while (true) {
        std::string input;
        std::cout << ">>>";
        std::cin >> input;
        if (input == "q") {
            break;
        } else if (input == "h") {
            std::cout << type_list << help;
            continue;
        } else if (input == "c") {
            std::cout << "generate canceled\n";
            std::cin.get();
            return;
        }
        uint32_t loc;
        try {
            loc = std::stoul(input);
        } catch (std::invalid_argument& e) {
            std::cout << "invalid input\n";
            continue;
        }
        std::cin >> input;
        auto it = get_format.find(input);
        if (it == get_format.end()) {
            std::cout << "invalid input\n";
            continue;
        }
        sstream << it->second.type << " D" << cnt;
        if (it->second.count > 1) {
            sstream << '[' << it->second.count << ']';
        }
        sstream << ";\n\t";
        vtarr.emplace_back(
            VertexType{it->second.format, loc, offset, it->second.size});
        offset += it->second.size;
        cnt++;
    }
    std::cout << "\nGenerating code:\n";
    std::cout
        << "struct Vertex final {\n\t" << sstream.str()
        << "\n\tstatic std::vector<vk::VertexInputAttributeDescription> "
           "getAttribute();\n\tstatic "
           "std::vector<vk::VertexInputBindingDescription> getBinding();\n};\n";
    std::cout
        << "std::vector<vk::VertexInputAttributeDescription> getAttribute() "
           "{\n\tstd::vector<vk::VertexInputAttributeDescription> attr("
        << cnt << ");\n\t";
    cnt = 0;
    for (auto& ref : vtarr) {
        std::cout << "attr[" << cnt << "].setBinding(0)\n\t\t.setFormat("
                  << ref.format << ")\n\t\t.setLocation(" << ref.loc
                  << ")\n\t\t.setOffset(" << ref.offset << ");\n\t";
        cnt++;
    }
    std::cout << "return attr;\n}\n"
              << "std::vector<vk::VertexInputBindingDescription> "
                 "Vertex::getBinding() "
                 "{\n\tstd::vector<vk::VertexInputBindingDescription> bind("
              << cnt << ");\n\t";
    cnt = 0;
    for (auto& ref : vtarr) {
        std::cout << "bind[" << cnt
                  << "].setBinding(0)\n\t\t.setInputRate(vk::VertexInputRate::"
                     "eVertex)\n\t\t.setStride("
                  << ref.stride << ");\n\t";
        cnt++;
    }
    std::cout << "return bind;\n}" << std::endl;
    std::cin.get();
    return;
}

enum EPOrder { EPHelp, EPQuit, EPCREATE, EPDELETE, EPSHOW, EPCompute};
struct EPMepInfo {
    MExpression<float> epMEp;
    std::string epStr;
};
void expressionProcess() {
    static const char* ep_menu =
        "MExpression:compute use float\n0.h/help: get help\n1.q/quit: "
        "quit\n2.<c/create> <name> "
        "<expression>: create a expression\n3.<d/del> <name>: delete a "
        "expression\n4.<l/list> [name]:show all expressions\n5.<cp/compute> <name>:compute function\n";
    static const std::map<std::string, EPOrder, std::less<> > epget_order{
        {"q", EPQuit},   {"quit", EPQuit},     {"h", EPHelp}, {"help", EPHelp},
        {"c", EPCREATE}, {"create", EPCREATE}, {"l", EPSHOW}, {"list", EPSHOW},
        {"d", EPDELETE}, {"del", EPDELETE}, {"cp",EPCompute}, {"compute",EPCompute}};
    static std::map<std::string, EPMepInfo, std::less<> > ep_expressions;
    static int line_cnt = 0;
    std::string str;
    while (true) {
        std::cout << line_cnt++ << "\t>>";
        std::getline(std::cin, str);
        auto ed = str.begin(), st = ed;
        while (ed != str.end() && *ed != ' ')
            ed++;
        std::string_view sv(st, ed);
        auto pos = epget_order.find(sv);
        if (pos == epget_order.end()) {
            std::cout << "invalid order\n";
            continue;
        } else {
            if (pos->second == EPHelp) {
                std::cout << ep_menu;
            } else if (pos->second == EPQuit) {
                return;
            } else if (pos->second == EPCREATE) {
                if (ed == str.end()) {
                    std::cout << "no input\n";
                    continue;
                }
                st = ++ed;
                while (ed != str.end() && *ed != ' ')
                    ed++;
                std::string name(st, ed);
                auto& ep = ep_expressions[name];
                try {
                    ep.epStr = std::string(++ed, str.end());
                    ep.epMEp.complie(ep.epStr);
                } catch (const std::exception& e) {
                    std::cout << "expection in compile:" << e.what() << '\n';
                    ep_expressions.erase(name);
                }
                continue;
            } else if (pos->second == EPSHOW) {
                if (ed == str.end()) {
                    std::cout << "Number of stored expressions:"
                              << ep_expressions.size() << std::endl;
                    int i = 1;
                    for (const auto& it : ep_expressions) {
                        std::cout << i++ << ':' << it.first << '\t'
                                  << it.second.epStr << '\n';
                    }
                    continue;
                } else {
                    sv = std::string_view(ed + 1, str.end());
                    auto pos = ep_expressions.find(sv);
                    if (pos == ep_expressions.end()) {
                        std::cout << "Name<" << sv << "> not found.\n";
                        continue;
                    } else {
                        std::cout << sv << ':' << pos->second.epStr << '\n';
                        pos->second.epMEp._print();
                        continue;
                    }
                }
            } else if (pos->second == EPDELETE) {
                if (ed == str.end()) {
                    std::cout << "no input\n";
                    continue;
                }
                sv = std::string_view(ed + 1, str.end());
                auto pos = ep_expressions.find(sv);
                if (pos == ep_expressions.end()) {
                    std::cout << "Name<" << sv << "> not found.\n";
                    continue;
                } else {
                    ep_expressions.erase(pos);
                    std::cout << "Deleted\n";
                    continue;
                }
            } else if (pos->second == EPCompute) {
                if (ed == str.end()) {
                    std::cout << "no input\n";
                    continue;
                }
                sv = std::string_view(ed + 1, str.end());
                auto pos = ep_expressions.find(sv);
                if (pos == ep_expressions.end()) {
                    std::cout << "Name<" << sv << "> not found.\n";
                    continue;
                } else {
                    std::vector<float> args(pos->second.epMEp.argStr.size());
                    int i = 0;
                    for (auto& it : pos->second.epMEp.argStr) {
                        std::cout << "Argument " << it.first << ':';
                        float n;
                        std::cin >> n;
                        args[i] = n;i++;
                    }
                    std::cout << "Res:" << pos->second.epMEp.compute(args) << std::endl;
                    continue;
                }
            }
        }
    }
}

void shaderCreate(){
    static std::map<std::string,vk::ShaderStageFlagBits> stage {
        {"vertex",vk::ShaderStageFlagBits::eVertex},
        {"fragment",vk::ShaderStageFlagBits::eFragment},
        {"geometry",vk::ShaderStageFlagBits::eGeometry},
        {"tesscontrol",vk::ShaderStageFlagBits::eTessellationControl},
        {"tessevaluation",vk::ShaderStageFlagBits::eTessellationEvaluation}
    };
    std::cout << "stages:vertex,fragment,geometry,tesscontrol,tessevaluation\n";
    std::cout << "Create Shader:\nEnter save path(quit to cancel):";
    std::string path;
    std::cin >> path;
    if (path=="quit") return;
    std::cout << "Enter Files(type NULL to stop) then enter stage:";
    std::vector<std::string> data;
    std::vector<vk::ShaderStageFlagBits> sts;
    std::string tmp,tmp2;
    std::cin >> tmp >> tmp2;
    while (true)
    {
        if (tmp == "null" || tmp == "NULL") {
            break;
        } else if (tmp == "quit") {
            std::cout << "Canceled\n";
            return;
        }
        
        sts.push_back(stage[tmp2]);
        data.push_back(tmp);
        std::cout << "Enter:";
        std::cin >> tmp >> tmp2;
    }
    createShaderFile(path,data,sts);
    std::cout << "Create Success.\n";
}
void createShaderFile(const std::string& path,
                      const std::vector<std::string>& data,
                      const std::vector<vk::ShaderStageFlagBits>& stages){
    std::ofstream file(path,std::ios::binary|std::ios::trunc);
    uint32_t code = SHADER_HEAD_CODE;
    file.write((char*)&code,4);
    code = data.size();
    file.write((char*)&code,4);
    size_t st = file.tellp();
    file.seekp(sizeof(ShaderFileHead::Part)*data.size(),std::ios::cur);
    ShaderFileHead::Part write[data.size()];
    for (uint32_t i = 0; i < data.size(); i++)
    {
        std::ifstream in_file(data[i],std::ios::binary|std::ios::in|std::ios::ate);
        write[i].length = in_file.tellg();
        write[i].start = file.tellp();
        write[i].stage = stages[i];
        char* tmp = new char[write[i].length];
        in_file.seekg(std::ios::beg);
        in_file.read(tmp,write[i].length);
        file.write(tmp,write[i].length);
        delete tmp;
        in_file.close();
    }
    file.seekp(st);
    file.write((char*)write,sizeof(ShaderFileHead::Part)*data.size());
    file.close();
}
void modelCreate() {
    std::string path, outPath;
    std::cout << "Enter model file(type quit to quit):\n";
    if (path == "quit") {
        std::cout << "canceled\n";
        return;
    }
    std::getline(std::cin, path);
    std::cout << "Enter out file(type null to ignore or quit to quit):\n";
    std::getline(std::cin, outPath);
    if (outPath == "quit") {
        std::cout << "canceled\n";
        return;
    }
    if (outPath == "null"||outPath=="NULL")
        outPath = path;
    makeModelFile(path, outPath);
}