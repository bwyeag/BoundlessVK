#ifndef BOUNDLESS_SHADER_FILE
#define BOUNDLESS_SHADER_FILE
#include "init.hpp"
#include "types.hpp"
#include <string>
#include <fstream>
namespace BL {
class Shader {
    Vkpipelsha
    std::vector<VkShaderModule> modules;
    public:
    struct ShaderInfo {
        VkShaderStageFlagBits stage;
        uint32_t length;
        uint32_t *data;
    };
    Shader(const std::string& path);
    std::vector<ShaderInfo> readPartInfo(const std::string& path);

};
}  // namespace BL
#endif  //! BOUNDLESS_SHADER_FILE