#ifndef _BOUNDLESS_FILE_TYPES_FILE_
#define _BOUNDLESS_FILE_TYPES_FILE_
#include <cstdint>
#include <vulkan/vulkan.h>
namespace BL
{
/*
 * 文件格式定义
 */
struct Range {
    uint32_t offset, length; /*以byte计算的偏移及长度*/
};
const uint32_t SHADER_HEAD_CODE = 0x240501CF;
struct ShaderFileHead {
    struct Part {
        uint32_t start;
        uint32_t length;
        alignas(4) VkShaderStageFlagBits stage;
    };
    uint32_t _head;
    uint32_t partCount;
    Part parts[];
};
const uint32_t MESH_HEAD_CODE = 0x240720FE;
struct MeshFileHead {
    struct VertexAttr {
        alignas(4) VkFormat format;
        uint32_t location;
        uint32_t offset;
    };
    struct BufferInfo {
        uint32_t binding;
        uint32_t stride;
        alignas(4) VkVertexInputRate rate;
        Range data;  // 压缩数据
        Range attr;  // 以索引计算的参数
    };
    uint32_t _head;
    uint32_t nameLen;
    char name[64];
    VkPrimitiveTopology topology;
    VkIndexType indexType;
    uint32_t vertexCount;
    uint32_t indexCount; /*保证含有index*/
    uint32_t restartEnable;
    uint32_t restartIndex;

    Range vertexBuffers;  // 指向一些BufferInfo
    Range indexBuffer;    // 指向索引缓冲， 压缩
    Range vertexAttrs;    // 指向顶点参数

    std::string getName() {
        std::string res;
        res.resize(nameLen);
        memcpy(res.data(), name, nameLen);
        return res;
    }
    void setName(const char* n) {
        uint32_t i = 0;
        while (i < 64) {
            if (name[i] == '\0')
                break;
            else
                name[i] = n[i], i++;
        }
        nameLen = i;
        do {
            name[i] = '\0', i++;
        } while (i < 64);
    }
};
} // namespace BL
#endif //!_BOUNDLESS_FILE_TYPES_FILE_