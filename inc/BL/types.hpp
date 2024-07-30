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
    VkCommandPool* getPointer() { return &handle; }
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
    VkRenderPass* getPointer() { return &handle; }
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
struct pipelineCreateInfosPack {
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
    pipelineCreateInfosPack() {
        set_create_infos();
        createInfo.basePipelineIndex = -1;
    }
    pipelineCreateInfosPack(const pipelineCreateInfosPack& other) noexcept {
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
class pipeline {
    VkPipeline handle = VK_NULL_HANDLE;

   public:
    pipeline() = default;
    pipeline(VkGraphicsPipelineCreateInfo& createInfo) { create(createInfo); }
    pipeline(VkComputePipelineCreateInfo& createInfo) { create(createInfo); }
    pipeline(pipeline&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    ~pipeline() {
        vkDestroyPipeline(context.vulkanInfo.device, handle, nullptr);
    }
    operator VkPipeline() { return handle; }
    VkPipeline* getPointer() { return &handle; }
    VkResult create(VkGraphicsPipelineCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        VkResult result = vkCreateGraphicsPipelines(
            graphicsBase::Base().Device(), VK_NULL_HANDLE, 1, &createInfo,
            nullptr, &handle);
        if (result) {
            print_error(
                "pipeline",
                "Failed to create a graphics pipeline! Code:", int32_t(result));
        }
        return result;
    }
    VkResult create(VkComputePipelineCreateInfo& createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        VkResult result = vkCreateComputePipelines(
            graphicsBase::Base().Device(), VK_NULL_HANDLE, 1, &createInfo,
            nullptr, &handle);
        if (result) {
            print_error(
                "pipeline",
                "Failed to create a graphics pipeline! Code:", int32_t(result));
        }

        return result;
    }
};
}  // namespace BL
#endif  //! BOUNDLESS_TYPES_FILE