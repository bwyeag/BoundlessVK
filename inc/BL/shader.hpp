#ifndef BOUNDLESS_SHADER_FILE
#define BOUNDLESS_SHADER_FILE
#include <exception>
#include <fstream>
#include <string>
#include <utility>
#include "ftypes.hpp"
#include "init.hpp"
#include "types.hpp"
namespace BL {
const char* const SHADER_ENTRY_NAME = "main";
class Shader {
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    std::vector<VkShaderModule> modules;

    struct ShaderInfo {
        VkShaderStageFlagBits stage;
        uint32_t length;
        uint32_t* data;
    };
    std::vector<ShaderInfo> readPartInfo(const std::string& path);

   public:
    Shader() = default;
    Shader(const std::string& path, VkSpecializationInfo* specInfo = nullptr) {
        create(path, specInfo);
    }
    Shader(Shader&& other) {
        modules = std::move(other.modules);
        stages = std::move(other.stages);
    }
    ~Shader() {
        for (uint32_t i = 0; i < modules.size(); i++) {
            vkDestroyShaderModule(CurContext().vulkanInfo.device, modules[i],
                                  nullptr);
        }
    }
    std::vector<VkPipelineShaderStageCreateInfo>& getStages() { return stages; }
    VkPipelineShaderStageCreateInfo* getStagePointer() { return stages.data(); }
    uint32_t getStageCount() { return stages.size(); }
    void create(const std::string& path,
                VkSpecializationInfo* specInfo = nullptr);
};
}  // namespace BL
#endif  //! BOUNDLESS_SHADER_FILE