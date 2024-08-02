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
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

   public:
    Buffer() = default;
    Buffer(VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& allocInfo) {
        allocate(createInfo, allocInfo);
    }
    Buffer(VkDeviceSize size,
           VkBufferCreateFlags vk_flag,
           VkBufferUsageFlagBits vk_usage,
           VmaAllocationCreateFlags vma_flag,
           VmaMemoryUsage vma_usage,
           VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        allocate(size, vk_flag, vk_usage, vma_flag, vma_usage, sharing_mode);
    }
    Buffer(Buffer&& other) noexcept {
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
    }
    operator VkBuffer() { return handle; }
    VkBuffer* getPointer() { return &handle; }
    operator VmaAllocation() { return allocation; }
    VmaAllocation getAllocation() { return allocation; }
    ~Buffer() {
        if (handle) {
            vmaDestroyBuffer(context.vulkanInfo.allocator, handle, allocation);
        }
        handle = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
    VkResult transfer_data(const void* pData,
                           VkDeviceSize length,
                           VkDeviceSize offset = 0) {
        return vmaCopyMemoryToAllocation(context.vulkanInfo.allocator, pData,
                                         allocation, offset, length);
    }
    VkResult retrieve_data(void* pData,
                           VkDeviceSize length,
                           VkDeviceSize offset = 0) {
        return vmaCopyAllocationToMemory(context.vulkanInfo.allocator,
                                         allocation, offset, pData, length);
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
                      VkBufferUsageFlagBits vk_usage,
                      VmaAllocationCreateFlags vma_flag,
                      VmaMemoryUsage vma_usage,
                      VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE) {
        VkBufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags = vk_flag,
            .size = size,
            .usage = vk_usage,
            .sharingMode = sharing_mode,
        };
        VmaAllocationCreateInfo allocInfo = {.flags = vma_flag,
                                             .usage = vma_usage};
        return allocate(createInfo, allocInfo);
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
        if (!handle)
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
}  // namespace BL
#endif  //! BOUNDLESS_TYPES_FILE