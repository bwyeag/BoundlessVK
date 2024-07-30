#include "shader.hpp"

namespace BL {
void Shader::create(const std::string& path, VkSpecializationInfo* specInfo) {
    std::vector<ShaderInfo> info = readPartInfo(path);
    if (info.size() == 0) {
        return;
    }
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    modules.resize(info.size());
    stages.resize(info.size());
    for (uint32_t i = 0; i < info.size(); i++) {
        create_info.codeSize = info[i].length;
        create_info.pCode = info[i].data;
        VkResult res = vkCreateShaderModule(context.vulkanInfo.device,
                                            &create_info, nullptr, &modules[i]);
        if (res != VK_SUCCESS) {
            print_error("Shader", "Shader module create failed! Code:", res);
            abort();
        }
        stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[i].pNext = nullptr;
        stages[i].flags = 0;
        stages[i].pName = SHADER_ENTRY_NAME;
        stages[i].module = modules[i];
        stages[i].pSpecializationInfo = specInfo;
        stages[i].stage = info[i].stage;
    }
    for (uint32_t j = 0; j < info.size(); j++) {
        free(info[j].data);
    }
}
std::vector<Shader::ShaderInfo> Shader::readPartInfo(const std::string& path) {
    std::vector<Shader::ShaderInfo> res;
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        print_error("Shader", "File not found!");
        throw std::runtime_error("File not found!");
    }
    uint32_t h_code;
    input.read((char*)&h_code, sizeof(h_code));
    if (h_code != SHADER_HEAD_CODE) {
        print_error("Shader", "File headcode error!");
        throw std::runtime_error("File headcode error!");
    }
    uint32_t count;
    input.read((char*)&count, sizeof(count));
    res.resize(count);
    ShaderFileHead::Part tmp;
    for (uint32_t i = 0; i < count; i++) {
        input.read((char*)&tmp, sizeof(tmp));
        res[i].stage = tmp.stage;
        res[i].length = tmp.length;
        res[i].data = (uint32_t*)malloc(tmp.length);
        if (!res[i].data) {
            print_error("Shader", "malloc() failed!");
            for (uint32_t j = 0; j < i; j++) {
                free(res[j].data);
            }
            res.resize(0);
            throw std::bad_alloc();
        }
        size_t pos = input.tellg();
        input.seekg(tmp.start);
        input.read((char*)res[i].data, tmp.length);
        input.seekg(pos);
    }
    return res;
}
}  // namespace BL
