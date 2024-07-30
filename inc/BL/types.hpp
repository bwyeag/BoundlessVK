#ifndef BOUNDLESS_TYPES_FILE
#define BOUNDLESS_TYPES_FILE
#include <cstdint>
#include <vulkan/vulkan.h>
#include "init.hpp"
#include "log.hpp"
namespace BL {
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
    VkFence* getPointer() {return &handle;}
    VkResult wait(uint64_t time = UINT64_MAX) const {
        VkResult result =
            vkWaitForFences(context.vulkanInfo.device, 1, &handle, false, time);
        if (result)
            print_error("Fence",
                        "Failed to wait for the fence! Code:", int32_t(result));
        return result;
    }
    VkResult reset() const {
        VkResult result = vkResetFences(context.vulkanInfo.device, 1, &handle);
        if (result)
            print_error("Fence", "Failed to reset for the fence! Code:",
                        int32_t(result));
        return result;
    }
    VkResult wait_and_reset(uint64_t time = UINT64_MAX) const {
        VkResult result = wait(time);
        result || (result = reset());
        return result;
    }
    VkResult status() const {
        VkResult result = vkGetFenceStatus(context.vulkanInfo.device, handle);
        if (result <
            0)  // vkGetFenceStatus(...)成功时有两种结果，所以不能仅仅判断result是否非0
            print_error("Fence", "Failed to get the status of the fence! Code:",
                        int32_t(result));
        return result;
    }
    VkResult create(VkFenceCreateInfo& createInfo) {
        VkResult result = vkCreateFence(context.vulkanInfo.device, &createInfo,
                                        nullptr, &handle);
        if (result)
            print_error("Fence",
                        "Failed to create a fence! Code:", int32_t(result));
        return result;
    }
    VkResult create(VkFenceCreateFlags flags = 0) {
        VkFenceCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = flags};
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
    VkSemaphore* getPointer() {return &handle;}
    VkResult create(VkSemaphoreCreateInfo& createInfo) {
        VkResult result = vkCreateSemaphore(context.vulkanInfo.device,
                                            &createInfo, nullptr, &handle);
        if (result)
            print_error("Semaphore",
                        "Failed to create a semaphore! Code:", int32_t(result));
        return result;
    }
    VkResult create() {
        VkSemaphoreCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        return create(createInfo);
    }
};
class commandBuffer {
    friend class commandPool;
    VkCommandBuffer handle = VK_NULL_HANDLE;

   public:
    commandBuffer() = default;
    commandBuffer(commandBuffer&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    operator VkCommandBuffer() { return handle; }
    VkCommandBuffer* getPointer() {return &handle;}
    VkResult begin(VkCommandBufferUsageFlags usageFlags,
                   VkCommandBufferInheritanceInfo& inheritanceInfo) const {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = usageFlags,
            .pInheritanceInfo = &inheritanceInfo};
        VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
        if (result) {
            print_error(
                "commandBuffer",
                "Failed to begin a command buffer! Code:", int32_t(result));
        }
        return result;
    }
    VkResult begin(VkCommandBufferUsageFlags usageFlags = 0) const {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = usageFlags,
        };
        VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
        if (result) {
            print_error(
                "commandBuffer",
                "Failed to begin a command buffer! Code:", int32_t(result));
        }
        return result;
    }
    VkResult end() const {
        VkResult result = vkEndCommandBuffer(handle);
        if (result) {
            print_error(
                "commandBuffer",
                "Failed to end a command buffer! Code:", int32_t(result));
        }
        return result;
    }
};
class commandPool {
    VkCommandPool handle = VK_NULL_HANDLE;

   public:
    commandPool() = default;
    commandPool(VkCommandPoolCreateInfo& createInfo) { create(createInfo); }
    commandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0) {
        create(queueFamilyIndex, flags);
    }
    commandPool(commandPool&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~commandPool() {
        vkDestroyCommandPool(context.vulkanInfo.device, handle, nullptr);
    }
    operator VkCommandPool() { return handle; }
    VkCommandPool* getPointer() {return &handle;}
    VkResult allocate_buffer(
        commandBuffer* pBuffer,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
        VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = handle,
            .level = level,
            .commandBufferCount = 1};
        VkResult result =
            vkAllocateCommandBuffers(context.vulkanInfo.device, &allocateInfo,
                                     (VkCommandBuffer*)pBuffer);
        if (result) {
            print_error("commandPool", "Failed to allocate", 1,
                        "command buffer(s)! Code:", int32_t(result));
        }
        return result;
    }
    VkResult allocate_buffers(
        commandBuffer* pBuffers,
        uint32_t count,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
        VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = handle,
            .level = level,
            .commandBufferCount = count};
        VkResult result =
            vkAllocateCommandBuffers(context.vulkanInfo.device, &allocateInfo,
                                     (VkCommandBuffer*)pBuffers);
        if (result) {
            print_error("commandPool", "Failed to allocate", count,
                        "command buffer(s)! Code:", int32_t(result));
        }
        return result;
    }
    void free_buffer(commandBuffer* pBuffer) const {
        vkFreeCommandBuffers(context.vulkanInfo.device, handle, 1,
                             (VkCommandBuffer*)pBuffer);
        pBuffer->handle = VK_NULL_HANDLE;
    }
    void free_buffers(commandBuffer* pBuffers, uint32_t count) const {
        vkFreeCommandBuffers(context.vulkanInfo.device, handle, count,
                             (VkCommandBuffer*)pBuffers);
        std::memset(pBuffers, 0, sizeof(VkCommandBuffer) * count);
    }
    VkResult create(VkCommandPoolCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        VkResult result = vkCreateCommandPool(context.vulkanInfo.device,
                                              &createInfo, nullptr, &handle);
        if (result) {
            print_error("commandPool", "Failed to create a command pool! Code:",
                        int32_t(result));
        }
        return result;
    }
    VkResult create(uint32_t queueFamilyIndex,
                    VkCommandPoolCreateFlags flags = 0) {
        VkCommandPoolCreateInfo createInfo = {
            .flags = flags, .queueFamilyIndex = queueFamilyIndex};
        return create(createInfo);
    }
};
class renderPass {
    VkRenderPass handle = VK_NULL_HANDLE;

   public:
    renderPass() = default;
    renderPass(VkRenderPassCreateInfo& createInfo) { create(createInfo); }
    renderPass(renderPass&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~renderPass() {
        vkDestroyRenderPass(context.vulkanInfo.device, handle, nullptr);
    }
    operator VkRenderPass() { return handle; }
    VkRenderPass* getPointer() {return &handle;}
    void cmd_begin(
        VkCommandBuffer commandBuffer,
        VkRenderPassBeginInfo& beginInfo,
        VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = handle;
        vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
    }
    void cmd_begin(
        VkCommandBuffer commandBuffer,
        VkFramebuffer framebuffer,
        VkRect2D renderArea,
        const VkClearValue* clearValues = nullptr,
        uint32_t clearValuesCount = 0,
        VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
        VkRenderPassBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = handle,
            .framebuffer = framebuffer,
            .renderArea = renderArea,
            .clearValueCount = clearValuesCount,
            .pClearValues = clearValues};
        vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
    }
    void cmd_next(
        VkCommandBuffer commandBuffer,
        VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
        vkCmdNextSubpass(commandBuffer, subpassContents);
    }
    void cmd_end(VkCommandBuffer commandBuffer) const {
        vkCmdEndRenderPass(commandBuffer);
    }
    VkResult create(VkRenderPassCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        VkResult result = vkCreateRenderPass(context.vulkanInfo.device,
                                             &createInfo, nullptr, &handle);
        if (result) {
            print_error("renderPass", "Failed to create a render pass! Code:",
                        int32_t(result));
        }
        return result;
    }
};
class framebuffer {
    VkFramebuffer handle = VK_NULL_HANDLE;

   public:
    framebuffer() = default;
    framebuffer(VkFramebufferCreateInfo& createInfo) { create(createInfo); }
    framebuffer(framebuffer&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
        }
    ~framebuffer() {
        vkDestroyFramebuffer(context.vulkanInfo.device, handle, nullptr);
    }
    operator VkFramebuffer() { return handle; }
    VkFramebuffer* getPointer() {return &handle;}
    VkResult create(VkFramebufferCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        VkResult result = vkCreateFramebuffer(context.vulkanInfo.device,
                                              &createInfo, nullptr, &handle);
        if (result)
            print_error("framebuffer", "Failed to create a framebuffer Code:",
                        int32_t(result));
        return result;
    }
};
}  // namespace BL
#endif  //! BOUNDLESS_TYPES_FILE