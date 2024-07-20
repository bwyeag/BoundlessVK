#ifndef BOUNDLESS_TYPES_FILE
#define BOUNDLESS_TYPES_FILE
#include <init.hpp>
namespace BL {
/*
* 文件格式定义
*/
struct Range {
    uint32_t offset,length;/*以byte计算的偏移及长度*/
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
        Range data;// 压缩数据
        Range attr;// 以索引计算的参数
    };
    uint32_t _head;
    uint32_t nameLen;
    char name[64];
    VkPrimitiveTopology topology;
    VkIndexType indexType;
    uint32_t vertexCount;
    uint32_t indexCount;/*保证含有index*/
    uint32_t restartEnable;
    uint32_t restartIndex;

    Range vertexBuffers;// 指向一些BufferInfo
    Range indexBuffer;// 指向索引缓冲， 压缩
    Range vertexAttrs;// 指向顶点参数

    std::string getName() {
        std::string res(nameLen);
        memcpy(res.data(),name,nameLen);
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
        } while (i<64);
    }
};
/*
* Vulkan类型封装
*/
class fence {
    VkFence handle = VK_NULL_HANDLE;

   public:
    fence(VkFenceCreateInfo& createInfo) { create(createInfo); }
    // 默认构造器创建未置位的栅栏
    fence(VkFenceCreateFlags flags = 0) { create(flags); }
    fence(fence&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~fence() { vkDestroyFence(context.vulkanInfo.device, handle, nullptr); }
    operator VkFence() { return handle; }
    result_t wait(uint64_t time = UINT64_MAX) const {
        VkResult result =
            vkWaitForFences(context.vulkanInfo.device, 1, &handle, false, time);
        if (result)
            print_error("Fence", "Failed to wait for the fence! Error code:",
                        int32_t(result));
        return result;
    }
    result_t reset() const {
        VkResult result = vkResetFences(context.vulkanInfo.device, 1, &handle);
        if (result)
            print_error("Fence", "Failed to reset for the fence! Error code:",
                        int32_t(result));
        return result;
    }
    result_t wait_and_reset() const {
        VkResult result = wait();
        result || (result = reset());
        return result;
    }
    result_t status() const {
        VkResult result = vkGetFenceStatus(context.vulkanInfo.device, handle);
        if (result <
            0)  // vkGetFenceStatus(...)成功时有两种结果，所以不能仅仅判断result是否非0
            print_error("Fence",
                        "Failed to get the status of the fence! Error code:",
                        int32_t(result));
        return result;
    }
    result_t create(VkFenceCreateInfo& createInfo) {
        VkResult result = vkCreateFence(context.vulkanInfo.device, &createInfo,
                                        nullptr, &handle);
        if (result)
            print_error("Fence", "Failed to create a fence! Error code:",
                        int32_t(result));
        return result;
    }
    result_t create(VkFenceCreateFlags flags = 0) {
        VkFenceCreateInfo createInfo = {
            .flags = flags, .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        return create(createInfo);
    }
};
class semaphore {
    VkSemaphore handle = VK_NULL_HANDLE;

   public:
    semaphore(VkSemaphoreCreateInfo& createInfo) { create(createInfo); }
    semaphore() { create(); }
    semaphore(semaphore&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~semaphore() {
        vkDestroySemaphore(context.vulkanInfo.device, handle, nullptr);
    }
    operator VkSemaphore() { return handle; }
    result_t create(VkSemaphoreCreateInfo& createInfo) {
        VkResult result = vkCreateSemaphore(context.vulkanInfo.device,
                                            &createInfo, nullptr, &handle);
        if (result)
            print_error(
                "Semaphore",
                "Failed to create a semaphore! Error code:", int32_t(result));
        return result;
    }
    result_t create() {
        VkSemaphoreCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        return create(createInfo);
    }
};
}  // namespace BL
#endif  //! BOUNDLESS_TYPES_FILE