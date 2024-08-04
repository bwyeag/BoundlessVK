#ifndef BOUNDLESS_TYPES_FILE
#define BOUNDLESS_TYPES_FILE
#include <vulkan/vulkan.h>
#include <cstdint>
#include "init.hpp"
#include "log.hpp"
namespace BL {
/*
 * Vulkan类型封装
 */
class Fence {
    VkFence handle = VK_NULL_HANDLE;

   public:
    Fence() = default;
    Fence(VkFenceCreateInfo& createInfo) { create(createInfo); }
    Fence(VkFenceCreateFlags flags) { create(flags); }
    Fence(Fence&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~Fence() {
        if (handle)
            vkDestroyFence(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkFence() { return handle; }
    VkFence* getPointer() { return &handle; }
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
        VkFenceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = flags};
        return create(createInfo);
    }
};
class Semaphore {
    VkSemaphore handle = VK_NULL_HANDLE;

   public:
    Semaphore() = default;
    Semaphore(VkSemaphoreCreateInfo& createInfo) { create(createInfo); }
    Semaphore(Semaphore&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~Semaphore() {
        if (handle)
            vkDestroySemaphore(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkSemaphore() { return handle; }
    VkSemaphore* getPointer() { return &handle; }
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
class CommandBuffer {
    friend class CommandPool;
    VkCommandBuffer handle = VK_NULL_HANDLE;

   public:
    CommandBuffer() = default;
    CommandBuffer(CommandBuffer&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    operator VkCommandBuffer() { return handle; }
    VkCommandBuffer* getPointer() { return &handle; }
    VkResult begin(VkCommandBufferUsageFlags usageFlags,
                   VkCommandBufferInheritanceInfo& inheritanceInfo) const {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = usageFlags,
            .pInheritanceInfo = &inheritanceInfo};
        VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
        if (result) {
            print_error(
                "CommandBuffer",
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
                "CommandBuffer",
                "Failed to begin a command buffer! Code:", int32_t(result));
        }
        return result;
    }
    VkResult end() const {
        VkResult result = vkEndCommandBuffer(handle);
        if (result) {
            print_error(
                "CommandBuffer",
                "Failed to end a command buffer! Code:", int32_t(result));
        }
        return result;
    }
};
class CommandPool {
    VkCommandPool handle = VK_NULL_HANDLE;

   public:
    CommandPool() = default;
    CommandPool(VkCommandPoolCreateInfo& createInfo) { create(createInfo); }
    CommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0) {
        create(queueFamilyIndex, flags);
    }
    CommandPool(CommandPool&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~CommandPool() {
        if (handle)
            vkDestroyCommandPool(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkCommandPool() { return handle; }
    VkCommandPool* getPointer() { return &handle; }
    VkResult allocate_buffer(
        CommandBuffer* pBuffer,
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
        CommandBuffer* pBuffers,
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
    void free_buffer(CommandBuffer* pBuffer) const {
        vkFreeCommandBuffers(context.vulkanInfo.device, handle, 1,
                             (VkCommandBuffer*)pBuffer);
        pBuffer->handle = VK_NULL_HANDLE;
    }
    void free_buffers(CommandBuffer* pBuffers, uint32_t count) const {
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
class RenderPass {
    VkRenderPass handle = VK_NULL_HANDLE;

   public:
    RenderPass() = default;
    RenderPass(VkRenderPassCreateInfo& createInfo) { create(createInfo); }
    RenderPass(RenderPass&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~RenderPass() {
        if (handle)
            vkDestroyRenderPass(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkRenderPass() { return handle; }
    VkRenderPass* getPointer() { return &handle; }
    void cmd_begin(
        VkCommandBuffer cmdBuf,
        VkRenderPassBeginInfo& beginInfo,
        VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
        beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.renderPass = handle;
        vkCmdBeginRenderPass(cmdBuf, &beginInfo, subpassContents);
    }
    void cmd_begin(
        VkCommandBuffer cmdBuf,
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
        vkCmdBeginRenderPass(cmdBuf, &beginInfo, subpassContents);
    }
    void cmd_next(
        VkCommandBuffer cmdBuf,
        VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
        vkCmdNextSubpass(cmdBuf, subpassContents);
    }
    void cmd_end(VkCommandBuffer cmdBuf) const { vkCmdEndRenderPass(cmdBuf); }
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
class Framebuffer {
    VkFramebuffer handle = VK_NULL_HANDLE;

   public:
    Framebuffer() = default;
    Framebuffer(VkFramebufferCreateInfo& createInfo) { create(createInfo); }
    Framebuffer(Framebuffer&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~Framebuffer() {
        if (handle)
            vkDestroyFramebuffer(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkFramebuffer() { return handle; }
    VkFramebuffer* getPointer() { return &handle; }
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
struct PipelineCreateInfosPack {
    VkGraphicsPipelineCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    // 顶点输入
    VkPipelineVertexInputStateCreateInfo vertexInputStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    std::vector<VkVertexInputBindingDescription> vertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
    // 顶点汇编
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    // 细分着色器状态
    VkPipelineTessellationStateCreateInfo tessellationStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    // 视口
    VkPipelineViewportStateCreateInfo viewportStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    // 动态视口/剪裁不会用到上述的vector，因此手动指定动态视口和剪裁的个数
    uint32_t dynamicViewportCount = 1;
    uint32_t dynamicScissorCount = 1;
    // 光栅化
    VkPipelineRasterizationStateCreateInfo rasterizationStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    // 多重采样
    VkPipelineMultisampleStateCreateInfo multisampleStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    // 深度 & 剪裁
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    // 颜色融混
    VkPipelineColorBlendStateCreateInfo colorBlendStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    // 动态管线状态
    VkPipelineDynamicStateCreateInfo dynamicStateCi = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    std::vector<VkDynamicState> dynamicStates;
    //--------------------
    PipelineCreateInfosPack() {
        set_create_infos();
        createInfo.basePipelineIndex = -1;
    }
    PipelineCreateInfosPack(const PipelineCreateInfosPack& other) noexcept {
        createInfo = other.createInfo;
        set_create_infos();

        vertexInputStateCi = other.vertexInputStateCi;
        inputAssemblyStateCi = other.inputAssemblyStateCi;
        tessellationStateCi = other.tessellationStateCi;
        viewportStateCi = other.viewportStateCi;
        rasterizationStateCi = other.rasterizationStateCi;
        multisampleStateCi = other.multisampleStateCi;
        depthStencilStateCi = other.depthStencilStateCi;
        colorBlendStateCi = other.colorBlendStateCi;
        dynamicStateCi = other.dynamicStateCi;

        shaderStages = other.shaderStages;
        vertexInputBindings = other.vertexInputBindings;
        vertexInputAttributes = other.vertexInputAttributes;
        viewports = other.viewports;
        scissors = other.scissors;
        colorBlendAttachmentStates = other.colorBlendAttachmentStates;
        dynamicStates = other.dynamicStates;
        update_all_array_pointers();
    }
    operator VkGraphicsPipelineCreateInfo&() { return createInfo; }
    VkGraphicsPipelineCreateInfo* getPointer() { return &createInfo; }
    // 该函数用于将各个vector中数据的地址赋值给各个创建信息中相应成员，并相应改变各个count
    void update_all_arrays() {
        createInfo.stageCount = shaderStages.size();
        vertexInputStateCi.vertexBindingDescriptionCount =
            vertexInputBindings.size();
        vertexInputStateCi.vertexAttributeDescriptionCount =
            vertexInputAttributes.size();
        viewportStateCi.viewportCount = viewports.size()
                                            ? uint32_t(viewports.size())
                                            : dynamicViewportCount;
        viewportStateCi.scissorCount =
            scissors.size() ? uint32_t(scissors.size()) : dynamicScissorCount;
        colorBlendStateCi.attachmentCount = colorBlendAttachmentStates.size();
        dynamicStateCi.dynamicStateCount = dynamicStates.size();
        update_all_array_pointers();
    }

   private:
    // 将创建信息的地址赋值给basePipelineIndex中相应成员
    void set_create_infos() {
        createInfo.pVertexInputState = &vertexInputStateCi;
        createInfo.pInputAssemblyState = &inputAssemblyStateCi;
        createInfo.pTessellationState = &tessellationStateCi;
        createInfo.pViewportState = &viewportStateCi;
        createInfo.pRasterizationState = &rasterizationStateCi;
        createInfo.pMultisampleState = &multisampleStateCi;
        createInfo.pDepthStencilState = &depthStencilStateCi;
        createInfo.pColorBlendState = &colorBlendStateCi;
        createInfo.pDynamicState = &dynamicStateCi;
    }
    // 该将各个vector中数据的地址赋值给各个创建信息中相应成员，但不改变各个count
    void update_all_array_pointers() {
        createInfo.pStages = shaderStages.data();
        vertexInputStateCi.pVertexBindingDescriptions =
            vertexInputBindings.data();
        vertexInputStateCi.pVertexAttributeDescriptions =
            vertexInputAttributes.data();
        viewportStateCi.pViewports = viewports.data();
        viewportStateCi.pScissors = scissors.data();
        colorBlendStateCi.pAttachments = colorBlendAttachmentStates.data();
        dynamicStateCi.pDynamicStates = dynamicStates.data();
    }
};
class PipelineLayout {
    VkPipelineLayout handle = VK_NULL_HANDLE;

   public:
    PipelineLayout() = default;
    PipelineLayout(VkPipelineLayoutCreateInfo& createInfo) {
        create(createInfo);
    }
    PipelineLayout(PipelineLayout&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~PipelineLayout() {
        if (handle)
            vkDestroyPipelineLayout(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkPipelineLayout() { return handle; }
    VkPipelineLayout* getPointer() { return &handle; }

    VkResult create(VkPipelineLayoutCreateInfo& createInfo) {
        VkResult result = vkCreatePipelineLayout(context.vulkanInfo.device,
                                                 &createInfo, nullptr, &handle);
        if (result) {
            print_error(
                "pipelineLayout",
                "create pipelineLayout failed! Code: ", int32_t(result));
        }
        return result;
    }
};
class Pipeline {
    VkPipeline handle = VK_NULL_HANDLE;

   public:
    Pipeline() = default;
    Pipeline(VkGraphicsPipelineCreateInfo& createInfo) { create(createInfo); }
    Pipeline(VkComputePipelineCreateInfo& createInfo) { create(createInfo); }
    Pipeline(Pipeline&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~Pipeline() {
        vkDestroyPipeline(context.vulkanInfo.device, handle, nullptr);
    }
    operator VkPipeline() { return handle; }
    VkPipeline* getPointer() { return &handle; }
    VkResult create(VkGraphicsPipelineCreateInfo& createInfo) {
        VkResult result =
            vkCreateGraphicsPipelines(context.vulkanInfo.device, VK_NULL_HANDLE,
                                      1, &createInfo, nullptr, &handle);
        if (result) {
            print_error(
                "pipeline",
                "Failed to create a graphics pipeline! Code:", int32_t(result));
        }
        return result;
    }
    VkResult create(VkComputePipelineCreateInfo& createInfo) {
        VkResult result =
            vkCreateComputePipelines(context.vulkanInfo.device, VK_NULL_HANDLE,
                                     1, &createInfo, nullptr, &handle);
        if (result) {
            print_error(
                "pipeline",
                "Failed to create a compute pipeline! Code:", int32_t(result));
        }

        return result;
    }
};
class Buffer {
   protected:
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

   public:
    Buffer() = default;
    Buffer(VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo) {
        allocate(createInfo, allocInfo);
    }
    Buffer(VkDeviceSize size,
           VkBufferCreateFlags vk_flag,
           VkBufferUsageFlags vk_usage,
           VmaAllocationCreateFlags vma_flag,
           VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_AUTO,
           VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        allocate(size, vk_flag, vk_usage, vma_flag, vma_usage, sharing_mode);
    }
    Buffer(Buffer&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
        allocation = other.allocation;
        other.allocation = VK_NULL_HANDLE;
    }
    operator VkBuffer() { return handle; }
    VkBuffer* getPointer() { return &handle; }
    operator VmaAllocation() { return allocation; }
    VmaAllocation getAllocation() { return allocation; }
    ~Buffer() {
        vmaDestroyBuffer(context.vulkanInfo.allocator, handle, allocation);
        handle = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
    VkResult transfer_data(const void* pData,
                           VkDeviceSize length,
                           VkDeviceSize offset = 0) {
        VkResult result = vmaCopyMemoryToAllocation(
            context.vulkanInfo.allocator, pData, allocation, offset, length);
        if (result) {
            print_error("Buffer",
                        "transfer_data() failed! Code:", int32_t(result));
        }
        return result;
    }
    VkResult retrieve_data(void* pData,
                           VkDeviceSize length,
                           VkDeviceSize offset = 0) {
        VkResult result = vmaCopyAllocationToMemory(
            context.vulkanInfo.allocator, allocation, offset, pData, length);
        if (result) {
            print_error("Buffer",
                        "retrieve_data() failed! Code:", int32_t(result));
        }
        return result;
    }
    void* map_data() {
        void* data = nullptr;
        VkResult result =
            vmaMapMemory(context.vulkanInfo.allocator, allocation, &data);
        if (result) {
            print_error("Buffer", "map_data() failed! Code:", int32_t(result));
        }
        return data;
    }
    void unmap_data() {
        vmaUnmapMemory(context.vulkanInfo.allocator, allocation);
    }
    VkResult flush_data(VkDeviceSize offset = 0,
                        VkDeviceSize length = VK_WHOLE_SIZE) {
        VkResult result = vmaFlushAllocation(context.vulkanInfo.allocator,
                                             allocation, offset, length);
        if (result) {
            print_error("Buffer",
                        "flush_data() failed! Code:", int32_t(result));
        }
        return result;
    }
    VkResult invalidate_data(VkDeviceSize offset = 0,
                             VkDeviceSize length = VK_WHOLE_SIZE) {
        VkResult result = vmaInvalidateAllocation(context.vulkanInfo.allocator,
                                                  allocation, offset, length);
        if (result) {
            print_error("Buffer",
                        "invalidate_data() failed! Code:", int32_t(result));
        }
        return result;
    }
    VkResult allocate(VkBufferCreateInfo& createInfo,
                      VmaAllocationCreateInfo& allocInfo) {
        VkResult result =
            vmaCreateBuffer(context.vulkanInfo.allocator, &createInfo,
                            &allocInfo, &handle, &allocation, nullptr);
        if (result) {
            print_error("Buffer",
                        "VMA error when create Buffer. Code:", int32_t(result));
        }
        return result;
    }
    VkResult allocate(VkDeviceSize size,
                      VkBufferCreateFlags vk_flag,
                      VkBufferUsageFlags vk_usage,
                      VmaAllocationCreateFlags vma_flag,
                      VmaMemoryUsage vma_usage,
                      VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        VkBufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = vk_flag,
            .size = size,
            .usage = vk_usage,
            .sharingMode = sharing_mode};
        VmaAllocationCreateInfo allocInfo = {.flags = vma_flag,
                                             .usage = vma_usage};
        return allocate(createInfo, allocInfo);
    }
};
inline VkDeviceSize calculate_block_alignment(VkDeviceSize size) {
    static const VkDeviceSize uniformAlignment =
        context.vulkanInfo.phyDeviceProperties.limits
            .minUniformBufferOffsetAlignment;
    return ((uniformAlignment + size - 1) & ~(uniformAlignment - 1));
}
class IndexBuffer : protected Buffer {
   public:
    IndexBuffer() = default;
    IndexBuffer(VkDeviceSize block_size,
                VkBufferCreateFlags flags = 0,
                VkBufferUsageFlags other_usage = 0,
                VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        create(block_size, flags, other_usage, sharing_mode);
    }
    IndexBuffer(IndexBuffer&& other) noexcept : Buffer(std::move(other)) {}
    operator VkBuffer() { return handle; }
    VkBuffer* getPointer() { return &handle; }
    ~IndexBuffer() {}
    VkResult create(VkDeviceSize block_size,
                    VkBufferCreateFlags flags = 0,
                    VkBufferUsageFlags other_usage = 0,
                    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        VkResult result = this->allocate(
            block_size, flags,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | other_usage,
            0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, sharing_mode);
        return result;
    }
};
class VertexBuffer : protected Buffer {
   public:
    VertexBuffer() = default;
    VertexBuffer(VkDeviceSize block_size,
                 VkBufferCreateFlags flags = 0,
                 VkBufferUsageFlags other_usage = 0,
                 VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        create(block_size, flags, other_usage, sharing_mode);
    }
    VertexBuffer(VertexBuffer&& other) noexcept : Buffer(std::move(other)) {}
    operator VkBuffer() { return handle; }
    VkBuffer* getPointer() { return &handle; }
    ~VertexBuffer() {}
    VkResult create(VkDeviceSize block_size,
                    VkBufferCreateFlags flags = 0,
                    VkBufferUsageFlags other_usage = 0,
                    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        VkResult result = this->allocate(
            block_size, flags,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | other_usage,
            0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, sharing_mode);
        return result;
    }
};
class TransferBuffer : protected Buffer {
   protected:
    void* pBufferData;
    VkDeviceSize bufferSize;

   public:
    TransferBuffer() = default;
    TransferBuffer(VkDeviceSize block_size,
                   VkBufferCreateFlags flags = 0,
                   VkBufferUsageFlags other_usage = 0,
                   VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        create(block_size, flags, other_usage, sharing_mode);
    }
    TransferBuffer(TransferBuffer&& other) noexcept
        : Buffer(std::move(other)) {}
    operator VkBuffer() { return handle; }
    VkBuffer* getPointer() { return &handle; }
    void* get_pdata() { return pBufferData; }
    ~TransferBuffer() {}
    VkResult resize(VkDeviceSize new_size,
                    VkBufferCreateFlags flags = 0,
                    VkBufferUsageFlags other_usage = 0,
                    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        if (bufferSize >= new_size)
            return VK_SUCCESS;
        else {
            vmaDestroyBuffer(context.vulkanInfo.allocator, handle, allocation);
            return create(new_size, flags, other_usage, sharing_mode);
        }
    }
    VkResult flush() { return this->flush_data(); }
    VkResult flush(VkDeviceSize offset, VkDeviceSize length) {
        return this->flush_data(offset, length);
    }
    VkResult transfer_data(const void* pData) {
        memcpy(pBufferData, pData, bufferSize);
        return this->flush_data();
    }
    VkResult transfer_data(const void* pData,
                           VkDeviceSize offset,
                           VkDeviceSize length) {
        memcpy((uint8_t*)pBufferData + offset, pData, length);
        return this->flush_data(offset, length);
    }
    void cmd_insert_transfer(VkCommandBuffer cmdBuf,
                             VkBuffer dstBuf,
                             const VkBufferCopy* copyInfos,
                             uint32_t count = 1) {
        vkCmdCopyBuffer(cmdBuf, handle, dstBuf, count, copyInfos);
    }
    VkResult transfer_to_buffer(VkQueue cmdPool,
                                VkCommandBuffer cmdBuf,
                                VkBuffer dstBuf,
                                const VkBufferCopy* copyInfos,
                                uint32_t count,
                                VkFence fence) {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
        VkResult result = vkBeginCommandBuffer(cmdBuf, &beginInfo);
        if (result) {
            print_error(
                "TransferBuffer",
                "Failed to begin a command buffer! Code:", int32_t(result));
            return result;
        }
        vkCmdCopyBuffer(cmdBuf, handle, dstBuf, count, copyInfos);
        result = vkEndCommandBuffer(cmdBuf);
        if (result) {
            print_error(
                "TransferBuffer",
                "Failed to end a command buffer! Code:", int32_t(result));
            return result;
        }
        VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                   .commandBufferCount = 1,
                                   .pCommandBuffers = &cmdBuf};
        result = vkQueueSubmit(cmdPool, 1, &submitInfo, fence);
        if (result) {
            print_error(
                "TransferBuffer",
                "Failed to submit command! Code:", int32_t(result));
            return result;
        }
        return VK_SUCCESS;
    }
    VkResult create(VkDeviceSize block_size,
                    VkBufferCreateFlags flags = 0,
                    VkBufferUsageFlags other_usage = 0,
                    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        VkResult result = this->allocate(
            block_size, flags, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | other_usage,
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST, sharing_mode);
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(context.vulkanInfo.allocator, allocation,
                             &allocInfo);
        pBufferData = allocInfo.pMappedData;
        bufferSize = allocInfo.size;
        return result;
    }
};
class UniformBuffer : protected Buffer {
   protected:
    void* pBufferData;
    VkDeviceSize blockOffset, blockSize;

   public:
    UniformBuffer() = default;
    UniformBuffer(VkDeviceSize block_size,
                  VkBufferCreateFlags flags = 0,
                  VkBufferUsageFlags other_usage = 0,
                  VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        create(blockSize, flags, other_usage, sharing_mode);
    }
    UniformBuffer(UniformBuffer&& other) noexcept : Buffer(std::move(other)) {
        pBufferData = other.pBufferData;
        other.pBufferData = nullptr;
        blockOffset = other.blockOffset;
        blockSize = other.blockSize;
    }
    operator VkBuffer() { return handle; }
    VkBuffer* getPointer() { return &handle; }
    ~UniformBuffer() { pBufferData = nullptr; }
    void transfer_data(const void* pData) {
        for (uint32_t i = 0; i < MAX_FLIGHT_NUM; i++) {
            memcpy((uint8_t*)pBufferData + i * blockOffset, pData, blockSize);
        }
        this->flush_data();
    }
    void* get_pdata() { return pBufferData; }
    VkDeviceSize get_alignment() { return blockOffset; }
    VkDeviceSize get_block_size() { return blockSize; }

    VkResult create(VkDeviceSize block_size,
                    VkBufferCreateFlags flags = 0,
                    VkBufferUsageFlags other_usage = 0,
                    VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        blockOffset = calculate_block_alignment(block_size);
        blockSize = block_size;
        VkResult result = this->allocate(
            blockOffset * MAX_FLIGHT_NUM, flags,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | other_usage,
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO, sharing_mode);
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(context.vulkanInfo.allocator, allocation,
                             &allocInfo);
        pBufferData = allocInfo.pMappedData;
        return result;
    }
};
class BufferView {
    VkBufferView handle = VK_NULL_HANDLE;

   public:
    BufferView() = default;
    BufferView(VkBufferViewCreateInfo& createInfo) { create(createInfo); }
    BufferView(VkBuffer buffer,
               VkFormat format,
               VkDeviceSize offset = 0,
               VkDeviceSize range = 0 /*VkBufferViewCreateFlags flags*/) {
        create(buffer, format, offset, range);
    }
    BufferView(BufferView&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~BufferView() {
        if (handle)
            vkDestroyBufferView(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkBufferView() { return handle; }
    VkBufferView* getPointer() { return &handle; }
    VkResult create(VkBufferViewCreateInfo& createInfo) {
        VkResult result = vkCreateBufferView(context.vulkanInfo.device,
                                             &createInfo, nullptr, &handle);
        if (result)
            print_error("BufferView", "Failed to create a buffer view! Code:",
                        int32_t(result));
        return result;
    }
    VkResult create(VkBuffer buffer,
                    VkFormat format,
                    VkDeviceSize offset = 0,
                    VkDeviceSize range = 0 /*VkBufferViewCreateFlags flags*/) {
        VkBufferViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .buffer = buffer,
            .format = format,
            .offset = offset,
            .range = range};
        return create(createInfo);
    }
};
// 图像数据类
// 指定格式时使用 vk_format_utils.h 的内容
class Image {
    VkImage handle = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

   public:
    Image() = default;
    Image(VkImageCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo) {
        create(createInfo, allocInfo);
    }
    Image(Image&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
        allocation = other.allocation;
        other.allocation = VK_NULL_HANDLE;
    }
    ~Image() {
        vmaDestroyImage(context.vulkanInfo.allocator, handle, allocation);
        handle = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
    operator VkImage() { return handle; }
    VkImage* getPointer() { return &handle; }
    operator VmaAllocation() { return allocation; }
    VmaAllocation getAllocation() { return allocation; }
    VkResult create(VkImageCreateInfo& createInfo,
                    VmaAllocationCreateInfo& allocInfo) {
        VkResult result =
            vmaCreateImage(context.vulkanInfo.allocator, &createInfo,
                           &allocInfo, &handle, &allocation, nullptr);
        if (result)
            print_error("image",
                        "Failed to create an image! Code:", int32_t(result));
        return result;
    }
};
class ImageView {
    VkImageView handle = VK_NULL_HANDLE;

   public:
    ImageView() = default;
    ImageView(VkImageViewCreateInfo& createInfo) { allocate(createInfo); }
    ImageView(VkImage image,
              VkImageViewType viewType,
              VkFormat format,
              const VkImageSubresourceRange& subresourceRange,
              VkImageViewCreateFlags flags = 0) {
        allocate(image, viewType, format, subresourceRange, flags);
    }
    ImageView(ImageView&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~ImageView() {
        if (handle)
            vkDestroyImageView(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkImageView() { return handle; }
    VkImageView* getPointer() { return &handle; }
    VkResult allocate(VkImageViewCreateInfo& createInfo) {
        VkResult result = vkCreateImageView(context.vulkanInfo.device,
                                            &createInfo, nullptr, &handle);
        if (result)
            print_error("ImageView",
                        "Failed to create an image view! "
                        "Code:",
                        int32_t(result));
        return result;
    }
    VkResult allocate(VkImage image,
                      VkImageViewType viewType,
                      VkFormat format,
                      const VkImageSubresourceRange& subresourceRange,
                      VkImageViewCreateFlags flags = 0) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .flags = flags,
            .image = image,
            .viewType = viewType,
            .format = format,
            .subresourceRange = subresourceRange};
        return allocate(createInfo);
    }
};
class Sampler {
    VkSampler handle = VK_NULL_HANDLE;

   public:
    Sampler() = default;
    Sampler(VkSamplerCreateInfo& createInfo) { create(createInfo); }
    Sampler(Sampler&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~Sampler() {
        if (handle)
            vkDestroySampler(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkSampler() { return handle; }
    VkSampler* getPointer() { return &handle; }
    VkResult create(VkSamplerCreateInfo& createInfo) {
        VkResult result = vkCreateSampler(context.vulkanInfo.device,
                                          &createInfo, nullptr, &handle);
        if (result) {
            print_error("Sampler",
                        "Failed to create a Sampler! Code:", int32_t(result));
        }
        return result;
    }
};
class DescriptorSetLayout {
    VkDescriptorSetLayout handle = VK_NULL_HANDLE;

   public:
    DescriptorSetLayout() = default;
    DescriptorSetLayout(VkDescriptorSetLayoutCreateInfo& createInfo) {
        create(createInfo);
    }
    DescriptorSetLayout(DescriptorSetLayout&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~DescriptorSetLayout() {
        if (handle)
            vkDestroyDescriptorSetLayout(context.vulkanInfo.device, handle,
                                         nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkDescriptorSetLayout() { return handle; }
    VkDescriptorSetLayout* getPointer() { return &handle; }
    VkResult create(VkDescriptorSetLayoutCreateInfo& createInfo) {
        VkResult result = vkCreateDescriptorSetLayout(
            context.vulkanInfo.device, &createInfo, nullptr, &handle);
        if (result)
            print_error("DescriptorSetLayout",
                        "Failed to create a descriptor set layout! Code:",
                        int32_t(result));
        return result;
    }
};
class DescriptorSet {
    friend class DescriptorPool;
    VkDescriptorSet handle = VK_NULL_HANDLE;

   public:
    DescriptorSet() = default;
    DescriptorSet(DescriptorSet&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    operator VkDescriptorSet() { return handle; }
    VkDescriptorSet* getPointer() { return &handle; }
    void write(const VkDescriptorImageInfo* pDescriptorImageInfos,
               uint32_t descriptorInfoCount,
               VkDescriptorType descriptorType,
               uint32_t dstBinding = 0,
               uint32_t dstArrayElement = 0) const {
        VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = handle,
            .dstBinding = dstBinding,
            .dstArrayElement = dstArrayElement,
            .descriptorCount = descriptorInfoCount,
            .descriptorType = descriptorType,
            .pImageInfo = pDescriptorImageInfos};
        update(&writeDescriptorSet);
    }
    void write(const VkDescriptorBufferInfo* pDescriptorBufferInfos,
               uint32_t descriptorInfoCount,
               VkDescriptorType descriptorType,
               uint32_t dstBinding = 0,
               uint32_t dstArrayElement = 0) const {
        VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = handle,
            .dstBinding = dstBinding,
            .dstArrayElement = dstArrayElement,
            .descriptorCount = descriptorInfoCount,
            .descriptorType = descriptorType,
            .pBufferInfo = pDescriptorBufferInfos};
        update(&writeDescriptorSet);
    }
    void write(const VkBufferView* pBufferViews,
               uint32_t descriptorInfoCount,
               VkDescriptorType descriptorType,
               uint32_t dstBinding = 0,
               uint32_t dstArrayElement = 0) const {
        VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = handle,
            .dstBinding = dstBinding,
            .dstArrayElement = dstArrayElement,
            .descriptorCount = descriptorInfoCount,
            .descriptorType = descriptorType,
            .pTexelBufferView = pBufferViews};
        update(&writeDescriptorSet);
    }
    static void update(VkWriteDescriptorSet* write) {
        vkUpdateDescriptorSets(context.vulkanInfo.device, 1, write, 0, nullptr);
    }
    static void update(VkWriteDescriptorSet* write, VkCopyDescriptorSet* copy) {
        vkUpdateDescriptorSets(context.vulkanInfo.device, 1, write, 1, copy);
    }
    static void update(uint32_t writeCount,
                       VkWriteDescriptorSet* writes,
                       uint32_t copiesCount = 0,
                       VkCopyDescriptorSet* copies = nullptr) {
        vkUpdateDescriptorSets(context.vulkanInfo.device, writeCount, writes,
                               copiesCount, copies);
    }
};
class DescriptorPool {
    VkDescriptorPool handle = VK_NULL_HANDLE;

   public:
    DescriptorPool() = default;
    DescriptorPool(const VkDescriptorPoolCreateInfo& createInfo) {
        create(createInfo);
    }
    DescriptorPool(uint32_t maxSetCount,
                   uint32_t poolSizeCount,
                   const VkDescriptorPoolSize* poolSizes,
                   VkDescriptorPoolCreateFlags flags = 0) {
        create(maxSetCount, poolSizeCount, poolSizes, flags);
    }
    DescriptorPool(DescriptorPool&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~DescriptorPool() {
        if (handle) {
            vkDestroyDescriptorPool(context.vulkanInfo.device, handle, nullptr);
        }
        handle = VK_NULL_HANDLE;
    }
    operator VkDescriptorPool() { return handle; }
    VkDescriptorPool* getPointer() { return &handle; }
    // 分配描述符集
    // setLayouts的数量必须等于sets的数量
    VkResult allocate_sets(uint32_t setCount,
                           VkDescriptorSet* sets,
                           const VkDescriptorSetLayout* setLayouts) const {
        VkDescriptorSetAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = handle,
            .descriptorSetCount = setCount,
            .pSetLayouts = setLayouts};
        VkResult result = vkAllocateDescriptorSets(context.vulkanInfo.device,
                                                   &allocateInfo, sets);
        if (result) {
            print_error("DescriptorPool",
                        "Failed to allocate descriptor "
                        "sets! Code:",
                        int32_t(result));
        }
        return result;
    }
    VkResult free_sets(uint32_t setCount, VkDescriptorSet* sets) const {
        VkResult result = vkFreeDescriptorSets(context.vulkanInfo.device,
                                               handle, setCount, sets);
        memset(sets, 0, setCount * sizeof(VkDescriptorSet));
        return result;
    }
    VkResult create(const VkDescriptorPoolCreateInfo& createInfo) {
        VkResult result = vkCreateDescriptorPool(context.vulkanInfo.device,
                                                 &createInfo, nullptr, &handle);
        if (result)
            print_error("DescriptorPool",
                        "Failed to create a descriptor "
                        "pool! Code:",
                        int32_t(result));
        return result;
    }
    VkResult create(uint32_t maxSetCount,
                    uint32_t poolSizeCount,
                    const VkDescriptorPoolSize* poolSizes,
                    VkDescriptorPoolCreateFlags flags = 0) {
        VkDescriptorPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = flags,
            .maxSets = maxSetCount,
            .poolSizeCount = poolSizeCount,
            .pPoolSizes = poolSizes};
        return create(createInfo);
    }
};
class QueryPool {
    VkQueryPool handle = VK_NULL_HANDLE;

   public:
    QueryPool() = default;
    QueryPool(VkQueryPoolCreateInfo& createInfo) { create(createInfo); }
    QueryPool(VkQueryType queryType,
              uint32_t queryCount,
              VkQueryPipelineStatisticFlags pipelineStatistics =
                  0 /*VkQueryPoolCreateFlags flags*/) {
        create(queryType, queryCount, pipelineStatistics);
    }
    QueryPool(QueryPool&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~QueryPool() {
        if (handle)
            vkDestroyQueryPool(context.vulkanInfo.device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    operator VkQueryPool() { return handle; }
    VkQueryPool* getPointer() { return &handle; }
    void cmd_reset(VkCommandBuffer cmdBuf,
                   uint32_t firstQueryIndex,
                   uint32_t queryCount) const {
        vkCmdResetQueryPool(cmdBuf, handle, firstQueryIndex, queryCount);
    }
    void cmd_begin(VkCommandBuffer cmdBuf,
                   uint32_t queryIndex,
                   VkQueryControlFlags flags = 0) const {
        vkCmdBeginQuery(cmdBuf, handle, queryIndex, flags);
    }
    void cmd_end(VkCommandBuffer cmdBuf, uint32_t queryIndex) const {
        vkCmdEndQuery(cmdBuf, handle, queryIndex);
    }
    void cmd_write_timestamp(VkCommandBuffer cmdBuf,
                             VkPipelineStageFlagBits pipelineStage,
                             uint32_t queryIndex) const {
        vkCmdWriteTimestamp(cmdBuf, pipelineStage, handle, queryIndex);
    }
    void cmd_copy_results(VkCommandBuffer cmdBuf,
                          uint32_t firstQueryIndex,
                          uint32_t queryCount,
                          VkBuffer buffer_dst,
                          VkDeviceSize offset_dst,
                          VkDeviceSize stride,
                          VkQueryResultFlags flags = 0) const {
        vkCmdCopyQueryPoolResults(cmdBuf, handle, firstQueryIndex, queryCount,
                                  buffer_dst, offset_dst, stride, flags);
    }
    VkResult get_results(uint32_t firstQueryIndex,
                         uint32_t queryCount,
                         size_t dataSize,
                         void* pData_dst,
                         VkDeviceSize stride,
                         VkQueryResultFlags flags = 0) const {
        VkResult result = vkGetQueryPoolResults(
            context.vulkanInfo.device, handle, firstQueryIndex, queryCount,
            dataSize, pData_dst, stride, flags);
        if (result)
            result > 0
                ?  // 若返回值为VK_NOT_READY，则查询尚未结束，有查询结果尚不可获
                print_error("QueryPool", "Not all queries are available! Code:",
                            int32_t(result))
                : print_error("QueryPool",
                              "Failed to get query pool results! Code:",
                              int32_t(result));
        return result;
    }
    void reset(uint32_t firstQueryIndex, uint32_t queryCount) {
        vkResetQueryPool(context.vulkanInfo.device, handle, firstQueryIndex,
                         queryCount);
    }
    VkResult create(VkQueryPoolCreateInfo& createInfo) {
        VkResult result = vkCreateQueryPool(context.vulkanInfo.device,
                                            &createInfo, nullptr, &handle);
        if (result)
            print_error("QueryPool", "Failed to create a query pool! Code:",
                        int32_t(result));
        return result;
    }
    VkResult create(VkQueryType queryType,
                    uint32_t queryCount,
                    VkQueryPipelineStatisticFlags pipelineStatistics =
                        0 /*VkQueryPoolCreateFlags flags*/) {
        VkQueryPoolCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
            .queryType = queryType,
            .queryCount = queryCount,
            .pipelineStatistics = pipelineStatistics};
        return create(createInfo);
    }
};
class OcclusionQueries {
   protected:
    QueryPool queryPool;
    std::vector<uint32_t> occlusionResults;

   public:
    OcclusionQueries() = default;
    OcclusionQueries(uint32_t capacity) { create(capacity); }
    operator VkQueryPool() { return queryPool; }
    VkQueryPool* getPointer() { return queryPool.getPointer(); }
    uint32_t capacity() const { return occlusionResults.size(); }
    uint32_t passing_sample_count(uint32_t index) const {
        return occlusionResults[index];
    }
    void cmd_reset(VkCommandBuffer cmdBuf) const {
        queryPool.cmd_reset(cmdBuf, 0, capacity());
    }
    void cmd_begin(VkCommandBuffer cmdBuf,
                   uint32_t queryIndex,
                   bool isPrecise = false) const {
        queryPool.cmd_begin(cmdBuf, queryIndex, isPrecise);
    }
    void cmd_end(VkCommandBuffer cmdBuf, uint32_t queryIndex) const {
        queryPool.cmd_end(cmdBuf, queryIndex);
    }
    /*常用于GPU-driven遮挡剔除*/
    void cmd_copy_results(VkCommandBuffer cmdBuf,
                          uint32_t firstQueryIndex,
                          uint32_t queryCount,
                          VkBuffer buffer_dst,
                          VkDeviceSize offset_dst,
                          VkDeviceSize stride) const {
        // 需要等待查询结束以获取正确的数值，flags为VK_QUERY_RESULT_WAIT_BIT
        queryPool.cmd_copy_results(cmdBuf, firstQueryIndex, queryCount,
                                   buffer_dst, offset_dst, stride,
                                   VK_QUERY_RESULT_WAIT_BIT);
    }
    void create(uint32_t capacity) {
        occlusionResults.resize(capacity);
        occlusionResults.shrink_to_fit();
        queryPool.create(VK_QUERY_TYPE_OCCLUSION, capacity);
    }
    void recreate(uint32_t capacity) {
        waitAll();
        queryPool.~QueryPool();
        create(capacity);
    }
    VkResult get_results(uint32_t queryCount) {
        return queryPool.get_results(0, queryCount, queryCount * 4,
                                     occlusionResults.data(), 4);
    }
    VkResult get_results() {
        return queryPool.get_results(0, capacity(), capacity() * 4,
                                     occlusionResults.data(), 4);
    }
};
}  // namespace BL
#endif  //! BOUNDLESS_TYPES_FILE