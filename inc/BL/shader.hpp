#ifndef BOUNDLESS_SHADER_FILE
#define BOUNDLESS_SHADER_FILE
#include <fstream>
#include <string>
#include "init.hpp"
#include "types.hpp"
#include "ftypes.hpp"
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
    Shader(const std::string& path, VkSpecializationInfo* specInfo = nullptr);
    ~Shader();
    const std::vector<VkPipelineShaderStageCreateInfo>& getStages() const {
        return stages;
    }
};
}  // namespace BL
#endif  //! BOUNDLESS_SHADER_FILE