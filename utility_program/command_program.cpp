#include <vulkan/vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "BL/ftypes.hpp"
// command:
// g++ command_program.cpp -ID:\c++programs\BoundlessVK\BoundlessVK\inc
// -ID:\vulkanSDK\Include -O3 -oBLC
enum struct Order {
    Quit,
    ShowMenu,
    GeneVertexCode,
    ExpressionCompute,
    CreateShader,
    CreateModel
};
static const std::map<std::string, Order> to_order_enum{
    {"q", Order::Quit},
    {"quit", Order::Quit},
    {"h", Order::ShowMenu},
    {"help", Order::ShowMenu},
    {"v", Order::GeneVertexCode},
    {"vertex", Order::GeneVertexCode},
    {"ep", Order::ExpressionCompute},
    {"expression", Order::ExpressionCompute},
    {"s", Order::CreateShader},
    {"shader", Order::CreateShader},
    {"m", Order::CreateModel},
    {"model", Order::CreateModel}};
void print_menu();
void generate_VertexCode();
void generate_Shader();
void generate_Model();
void expression_compute();
void trim(std::string& s);
int main() {
    std::cout << "BL Command\n";
    // print_menu();
    std::string order;
    while (true) {
        std::cout << ">>";
        std::getline(std::cin, order);
        trim(order);
        auto it = to_order_enum.find(order);
        if (it == to_order_enum.end()) {
            std::cout << "invalid order:" << order << '\n';
        } else {
            switch (it->second) {
                case Order::Quit:
                    std::cout << "Quit." << std::endl;
                    exit(0);
                case Order::ShowMenu:
                    print_menu();
                    break;
                case Order::GeneVertexCode:
                    try {
                        generate_VertexCode();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
                case Order::CreateShader:
                    try {
                        generate_Shader();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
                case Order::CreateModel:

                    try {
                        generate_Model();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
                case Order::ExpressionCompute:
                    try {
                        expression_compute();
                    } catch (const std::exception& e) {
                        std::cerr << "exception rised:\n" << e.what() << '\n';
                    }
                    break;
            }
        }
        std::cin.ignore();
    }
}
void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
}
void tolower_str(std::string& s) {
    char* d = s.data();
    for (size_t i = 0; i < s.size(); i++) {
        d[i] = tolower(d[i]);
    }
}
void toupper_str(std::string& s) {
    char* d = s.data();
    for (size_t i = 0; i < s.size(); i++) {
        d[i] = toupper(d[i]);
    }
}
void print_menu() {
    static const char* str_menu =
        "0.q/quit:quit\n1.h/help: print menu\n2.v/vertex: vertex class "
        "code\n3.ep/expression: mathexpression compute\n4.s/shader: create "
        "shader file\n5.m/model: create model file\n";
    std::cout << str_menu;
}
void generate_VertexCode() {
    std::cout << "to do function.\n";
}
void createShaderFile(const std::string& path,
                      const std::vector<std::string>& data,
                      const std::vector<VkShaderStageFlagBits>& stages);
void generate_Shader() {
    static std::map<std::string, VkShaderStageFlagBits> stage{
        {"vertex", VK_SHADER_STAGE_VERTEX_BIT},
        {"fragment", VK_SHADER_STAGE_FRAGMENT_BIT},
        {"geometry", VK_SHADER_STAGE_GEOMETRY_BIT},
        {"tesscontrol", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
        {"tessevaluation", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT}};
    static std::map<VkShaderStageFlagBits, std::string> cmd_stage_str{
        {VK_SHADER_STAGE_VERTEX_BIT, "vertex"},
        {VK_SHADER_STAGE_FRAGMENT_BIT, "fragment"},
        {VK_SHADER_STAGE_GEOMETRY_BIT, "geometry"},
        {VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tesscontrol"},
        {VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tesseval"}};
    std::cout << "stages:vertex,fragment,geometry,tesscontrol,tessevaluation\n";
    std::cout << "Create Shader:\nEnter save path('quit' to cancel):";
    std::string path;
    std::getline(std::cin, path);
    trim(path);
    tolower_str(path);
    if (path == "quit")
        return;
    std::cout << "Enter File('null' to stop) paths:";
    std::vector<std::string> fpaths;
    std::vector<VkShaderStageFlagBits> stages;
    std::string fpath, fstage, tmp;
    while (true) {
        std::getline(std::cin, fpath);
        trim(fpath);
        // 检查是否为退出指令
        if (fpath.size() <= 4) {
            tmp = fpath;
            tolower_str(tmp);
            if (tmp == "null") {
                break;
            } else if (tmp == "quit") {
                std::cout << "Canceled\n";
                return;
            }
        }
        // 检查着色器类型
        VkShaderStageFlagBits cur_stage;
        if (fpath.find(".vert") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_VERTEX_BIT;
            std::cout << "auto identified file type:" << "vertex\n";
        } else if (fpath.find(".frag") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            std::cout << "auto identified file type:" << "fragment\n";
        } else if (fpath.find(".geom") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            std::cout << "auto identified file type:" << "geometry\n";
        } else if (fpath.find(".tesc") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            std::cout << "auto identified file type:"
                      << "tessellation_control\n";
        } else if (fpath.find(".tese") != std::string::npos) {
            cur_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            std::cout << "auto identified file type:"
                      << "tessellation_evaluation\n";
        } else {
            std::cout << "Enter Type:";
            std::getline(std::cin, fstage);
            trim(fstage);
            tolower_str(fstage);
            cur_stage = stage[fstage];
        }
        // 自动编译着色器
        static std::stringstream sscmd;
        size_t postfix = fpath.find(".spv");
        if (postfix == std::string::npos) {
            sscmd << "glslc.exe " << fpath << " -c -o" << fpath << ".spv"
                  << " -fshader-stage=" << cmd_stage_str[cur_stage];
            system(sscmd.str().c_str());
            sscmd.str("");
            fpath += ".spv";
            std::cout << "auto compile file to:" << fpath << '\n';
        }
        stages.push_back(cur_stage);
        fpaths.push_back(fpath);
        std::cout << "Enter:";
    }
    createShaderFile(path, fpaths, stages);
    std::cout << "Create Success.\n";
}
void createShaderFile(const std::string& save_path,
                      const std::vector<std::string>& fpaths,
                      const std::vector<VkShaderStageFlagBits>& stages) {
    std::ofstream fout(save_path, std::ios::binary | std::ios::trunc);
    uint32_t code = BL::SHADER_HEAD_CODE;
    fout.write((char*)&code, 4);
    code = fpaths.size();
    fout.write((char*)&code, 4);
    size_t st = fout.tellp();
    fout.seekp(sizeof(BL::ShaderFileHead::Part) * fpaths.size(), std::ios::cur);
    BL::ShaderFileHead::Part write[fpaths.size()];
    for (uint32_t i = 0; i < fpaths.size(); i++) {
        std::ifstream in_file(fpaths[i],
                              std::ios::binary | std::ios::in | std::ios::ate);
        write[i].length = in_file.tellg();
        write[i].start = fout.tellp();
        write[i].stage = stages[i];
        char* tmp = new char[write[i].length];
        in_file.seekg(std::ios::beg);
        in_file.read(tmp, write[i].length);
        fout.write(tmp, write[i].length);
        delete tmp;
        in_file.close();
    }
    fout.seekp(st);
    fout.write((char*)write, sizeof(BL::ShaderFileHead::Part) * fpaths.size());
    fout.close();
}
void generate_Model() {
    std::cout << "to do function.\n";
}
void expression_compute() {
    std::cout << "to do function.\n";
}