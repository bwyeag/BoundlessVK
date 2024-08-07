#include <render.hpp>
namespace BL {
RenderContext render_context;
VkExtent2D const& window_size =
    context.vulkanInfo.swapchainCreateInfo.imageExtent;
bool initVulkanRenderer() {
    VkResult result = _createRenderContext();
    if (result)
        return false;
    _createRenderLoop();
    return true;
}
void terminateVulkanRenderer() {
    _destroyRenderContext();
}
VkResult _createRenderContext() {
    auto& info = context.vulkanInfo;
    VkResult result;
    if (info.queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED) {
        result = render_context.cmdPool_graphics.create(
            info.queueFamilyIndex_graphics,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result)
            goto CREATE_FAILED;
        result = render_context.cmdPool_graphics.allocate_buffer(
            &render_context.cmdBuffer_transfer);
        if (result)
            goto CREATE_FAILED;
        result = render_context.cmdPool_graphics.allocate_buffers(
            render_context.cmdBufs.data(), MAX_FLIGHT_NUM);
        if (result)
            goto CREATE_FAILED;
    }
    if (info.queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED) {
        result = render_context.cmdPool_compute.create(
            info.queueFamilyIndex_presentation,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result)
            goto CREATE_FAILED;
    }
    // 由于可能的离屏渲染需要而特判
    if (info.queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
            info.queueFamilyIndex_presentation !=
                info.queueFamilyIndex_graphics &&
            info.swapchainCreateInfo.imageSharingMode ==
                VK_SHARING_MODE_EXCLUSIVE ||
        FORCE_OWNERSHIP_TRANSFER) {
        result = render_context.cmdPool_presentation.create(
            info.queueFamilyIndex_presentation,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result)
            goto CREATE_FAILED;
        result = render_context.cmdPool_presentation.allocate_buffers(
            render_context.cmdBuffer_presentation.data(), MAX_FLIGHT_NUM);
        if (result)
            goto CREATE_FAILED;
        for (auto& it : render_context.semsOwnershipIsTransfered)
            it.create();
        // 决定是否需要进行所有权转移
        render_context.ownership_transfer = true;
    }
    render_context.curFrame = 0;
    for (auto& it : render_context.fences)
        it.create(VK_FENCE_CREATE_SIGNALED_BIT);
    for (auto& it : render_context.semsImageAvaliable)
        it.create();
    for (auto& it : render_context.semsRenderFinish)
        it.create();
    return VK_SUCCESS;
CREATE_FAILED:
    _destroyRenderContext();
    print_error("_createRenderContext()", "initialize failed!");
    return result;
}
void _destroyRenderContext() {
    render_context.cmdPool_presentation.~CommandPool();
    render_context.cmdPool_graphics.~CommandPool();
    render_context.cmdPool_compute.~CommandPool();
    for (auto& it : render_context.fences)
        it.~Fence();
    for (auto& it : render_context.semsImageAvaliable)
        it.~Semaphore();
    for (auto& it : render_context.semsRenderFinish)
        it.~Semaphore();
    for (auto& it : render_context.semsOwnershipIsTransfered)
        it.~Semaphore();
}
VkResult submit_cmdBuffer_graphics_wait(VkCommandBuffer commandBuffer) {
    Fence fence;
    fence.create();
    VkResult result;
    result = submit_cmdBuffer_graphics(commandBuffer, VkFence(fence));
    if (result)
        return result;
    else
        return fence.wait();
}
VkResult submit_cmdBuffer_graphics(VkCommandBuffer commandBuffer,
                                   VkFence fence) {
    VkSubmitInfo submitInfo = {.commandBufferCount = 1,
                               .pCommandBuffers = &commandBuffer};
    VkResult result =
        vkQueueSubmit(context.vulkanInfo.queue_compute, 1, &submitInfo, fence);
    if (result) {
        print_error("submit_cmdBuffer_graphics",
                    "submit failed! Code:", int32_t(result));
        return result;
    }
    return VK_SUCCESS;
}
VkResult submit_cmdBuffer_compute_wait(VkCommandBuffer commandBuffer) {
    Fence fence;
    fence.create();
    VkResult result;
    result = submit_cmdBuffer_compute(commandBuffer, VkFence(fence));
    if (result)
        return result;
    else
        return fence.wait();
}
VkResult submit_cmdBuffer_compute(VkCommandBuffer commandBuffer,
                                  VkFence fence) {
    VkSubmitInfo submitInfo = {.commandBufferCount = 1,
                               .pCommandBuffers = &commandBuffer};
    VkResult result =
        vkQueueSubmit(context.vulkanInfo.queue_compute, 1, &submitInfo, fence);
    if (result) {
        print_error("submit_cmdBuffer_compute",
                    "submit failed! Code:", int32_t(result));
        return result;
    }
    return VK_SUCCESS;
}
void _insertCmd_transfer_image_ownership(VkCommandBuffer commandBuffer,
                                         uint32_t index) {
    VkImageMemoryBarrier imageMemoryBarrier_GraphicsToPresentation = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = context.vulkanInfo.queueFamilyIndex_graphics,
        .dstQueueFamilyIndex = context.vulkanInfo.queueFamilyIndex_presentation,
        .image = context.vulkanInfo.swapchainImages[index],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    vkCmdPipelineBarrier(
        commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
        &imageMemoryBarrier_GraphicsToPresentation);
}
VkResult submit_buffer_presentation(
    VkCommandBuffer commandBuffer,
    VkFence fence,
    VkSemaphore semaphore_renderingIsOver,
    VkSemaphore semaphore_ownershipIsTransfered) {
    const VkPipelineStageFlags waitDstStage =
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               .commandBufferCount = 1,
                               .pCommandBuffers = &commandBuffer};
    if (semaphore_renderingIsOver)
        submitInfo.waitSemaphoreCount = 1,
        submitInfo.pWaitSemaphores = &semaphore_renderingIsOver,
        submitInfo.pWaitDstStageMask = &waitDstStage;
    if (semaphore_ownershipIsTransfered)
        submitInfo.signalSemaphoreCount = 1,
        submitInfo.pSignalSemaphores = &semaphore_ownershipIsTransfered;
    VkResult result = vkQueueSubmit(context.vulkanInfo.queue_presentation, 1,
                                    &submitInfo, fence);
    if (result)
        print_error("submit_buffer_presentation",
                    "Failed to submit the presentation command buffer! Code:",
                    int32_t(result));
    return result;
}

void render(RenderDataPackBase& packet) {
    auto& vkInfo = context.vulkanInfo;
    auto& device = vkInfo.device;
    auto& swapchain = vkInfo.swapchain;
    auto& swapchainInfo = vkInfo.swapchainCreateInfo;
    RenderContext& loop = render_context;
    // 检查是否存在旧交换链，如果存在则销毁
    if (swapchainInfo.oldSwapchain && swapchainInfo.oldSwapchain != swapchain) {
        vkDestroySwapchainKHR(device, swapchainInfo.oldSwapchain, nullptr);
        swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    }
    // 等待当前帧的栅栏，确保在这一帧的命令已完成执行
    VkResult result = loop.fences[loop.curFrame].wait_and_reset();
    if (result != VK_SUCCESS) {
        print_error("render()", "wait for fence failed! Code:", result);
        loop.curFrame = (loop.curFrame + 1) % MAX_FLIGHT_NUM;
        return;
    }
    // 请求下一张图像，确保可用
    uint32_t image_index;
    acquire_next_image(&image_index, loop.semsImageAvaliable[loop.curFrame],
                       VK_NULL_HANDLE);
    auto& curBuf = loop.cmdBufs[loop.curFrame];
    result = vkResetCommandBuffer(VkCommandBuffer(curBuf), 0);
    if (result != VK_SUCCESS) {
        print_error("render()", "vkResetCommandBuffer() failed! Code:", result);
    }
    curBuf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    //----------------
    packet.pRenderFunction(curBuf, packet, image_index);
    //----------------
    if (render_context.ownership_transfer) {
        _insertCmd_transfer_image_ownership(curBuf, image_index);
    }
    curBuf.end();
    // 发送渲染命令
    VkPipelineStageFlags flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = loop.semsImageAvaliable[loop.curFrame].getPointer(),
        .pWaitDstStageMask = &flag,
        .commandBufferCount = 1,
        .pCommandBuffers = curBuf.getPointer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores =
            loop.ownership_transfer
                ? loop.semsOwnershipIsTransfered[loop.curFrame].getPointer()
                : loop.semsRenderFinish[loop.curFrame].getPointer()};
    result = vkQueueSubmit(context.vulkanInfo.queue_graphics, 1, &submit_info,
                           loop.ownership_transfer
                               ? VK_NULL_HANDLE
                               : VkFence(loop.fences[loop.curFrame]));
    if (result != VK_SUCCESS) {
        print_error("render()", "vkQueueSubmit() failed! Code:", result);
    }
    // 发送呈现命令
    if (loop.ownership_transfer) {
        loop.cmdBuffer_presentation[loop.curFrame].begin(
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        _insertCmd_transfer_image_ownership(
            loop.cmdBuffer_presentation[loop.curFrame], image_index);
        loop.cmdBuffer_presentation[loop.curFrame].end();
        submit_buffer_presentation(
            loop.cmdBuffer_presentation[loop.curFrame],
            loop.fences[loop.curFrame],
            loop.semsOwnershipIsTransfered[loop.curFrame],
            loop.semsRenderFinish[loop.curFrame]);
    }
    present_image(&image_index, 1,
                  loop.semsRenderFinish[loop.curFrame].getPointer());
    // 切换到下一帧
    loop.curFrame = (loop.curFrame + 1) % MAX_FLIGHT_NUM;
}
VkResult acquire_next_image(uint32_t* index,
                            VkSemaphore semsImageAvaliable,
                            VkFence fence) {
    VkExtent2D& t = context.vulkanInfo.swapchainCreateInfo.imageExtent;
    while (VkResult result = vkAcquireNextImageKHR(
               context.vulkanInfo.device, context.vulkanInfo.swapchain,
               UINT64_MAX, semsImageAvaliable, fence,
               index)) {  // 如果获取失败则重建交换链
        switch (result) {
            case VK_SUBOPTIMAL_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                recreateSwapchain();
                print_log("RenderSize", "New size:", t.width, t.height);
                break;
            default:
                print_error(
                    "acquire_next_image()",
                    "wait for image in swapchain failed! Code:", result);
                return result;
        }
    }
    return VK_SUCCESS;
}
VkResult present_image(uint32_t* index,
                       uint32_t waitSemsCount,
                       VkSemaphore* sems) {
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = waitSemsCount,
        .pWaitSemaphores = sems,
        .swapchainCount = 1,
        .pSwapchains = &context.vulkanInfo.swapchain,
        .pImageIndices = index};
    VkResult result =
        vkQueuePresentKHR(context.vulkanInfo.queue_presentation, &present_info);
    switch (result) {
        case VK_SUCCESS:
            break;
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
            recreateSwapchain();
            break;
        default:
            print_error("present_image()",
                        "Failed to queue the image for presentation! Code:",
                        int32_t(result));
            return result;
    }
    return VK_SUCCESS;
}
static int depth_attachment_callback_create;
static int depth_attachment_callback_destroy;
void initDepthAttachment() {
    render_context.depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    auto Create = []() {
        render_context.depthAttachments.resize(
            context.getSwapChainImageCount());
        for (auto& it : render_context.depthAttachments) {
            it.create(VK_FORMAT_D24_UNORM_S8_UINT, window_size);
        }
    };
    auto Destroy = []() { render_context.depthAttachments.clear(); };
    Create();
    depth_attachment_callback_create = addCallback_CreateSwapchain(Create);
    depth_attachment_callback_destroy = addCallback_DestroySwapchain(Destroy);
}
void destroyDepthAttachment() {
    removeCallback_CreateSwapchain(depth_attachment_callback_create);
    removeCallback_DestroySwapchain(depth_attachment_callback_destroy);
    render_context.depthAttachments.clear();
}
RenderPassPackBase::~RenderPassPackBase() {
    removeCallback_CreateSwapchain(callback_c_id);
    removeCallback_DestroySwapchain(callback_d_id);
}
RenderPipelineBase::~RenderPipelineBase() {
    removeCallback_CreateSwapchain(callback_c_id);
    removeCallback_DestroySwapchain(callback_d_id);
}
}  // namespace BL
