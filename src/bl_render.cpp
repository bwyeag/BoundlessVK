#include "bl_render.hpp"

namespace BL {
bool RenderContext::create(const RenderContextInit* pInit) {
    auto& info = CurContext();
    VkResult result;
    windowContext = pInit->windowContext;
    if (info.queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED) {
        result = cmdPool_graphics.create(
            info.queueFamilyIndex_graphics,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result)
            goto CREATE_FAILED;
        cmdBuffers.resize(MAX_FLIGHT_NUM * pInit->renderPassCount);
        result = cmdPool_graphics.allocate_buffers(cmdBuffers.data(),
                                                   cmdBuffers.size());
        if (result)
            goto CREATE_FAILED;
    }
    if (info.queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED) {
        result = cmdPool_compute.create(
            info.queueFamilyIndex_presentation,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result)
            goto CREATE_FAILED;
    }
    // 由于可能的离屏渲染需要而特判
    if ((info.queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
         info.queueFamilyIndex_presentation != info.queueFamilyIndex_graphics &&
         pInit->windowContext.swapchainCreateInfo.imageSharingMode ==
             VK_SHARING_MODE_EXCLUSIVE) ||
        pInit->force_ownership_transfer) {
        result = cmdPool_presentation.create(
            info.queueFamilyIndex_presentation,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result)
            goto CREATE_FAILED;
        result = cmdPool_presentation.allocate_buffers(
            cmdBuffer_presentation.data(), MAX_FLIGHT_NUM);
        if (result)
            goto CREATE_FAILED;
        for (auto& it : semsOwnershipIsTransfered)
            it.create();
        // 决定是否需要进行所有权转移
        ownership_transfer = true;
    }
    curFrame = 0;
    for (auto& it : fences)
        it.create(VK_FENCE_CREATE_SIGNALED_BIT);
    semaphores.resize((pInit->renderPassCount + 1) * MAX_FLIGHT_COUNT);
    currentRenderPass = curFrame = 0;
    maxRenderPassCount = pInit->renderPassCount;
    for (auto& it : semaphores)
        it.create();
    return VK_SUCCESS;
CREATE_FAILED:
    destroy();
    print_error("RenderContext", "Initialize failed! Code:", result);
    return result;
}
void RenderContext::destroy() {
    for (size_t i = 0; i < MAX_FLIGHT_NUM; i++) {
        std::destroy_at(&fences[i]);
        std::destroy_at(&semsOwnershipIsTransfered[i]);
        std::destroy_at(&cmdPool_graphics);
        std::destroy_at(&cmdPool_compute);
        std::destroy_at(&cmdPool_presentation);
    }
    semaphores.clear();
    windowContext = nullptr;
}
void RenderContext::beginRender() {
    auto& context = CurContext();
    auto& swapchain = windowContext->swapchain;
    auto& swapchainInfo = windowContext->swapchainCreateInfo;
    // 检查是否存在旧交换链，如果存在则销毁
    if (swapchainInfo.oldSwapchain && swapchainInfo.oldSwapchain != swapchain) {
        vkDestroySwapchainKHR(context.device, swapchainInfo.oldSwapchain,
                              nullptr);
        swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    }
    // 等待当前帧的栅栏，确保在这一帧的命令已完成执行
    VkResult result = fences[curFrame].wait_and_reset();
    if (result != VK_SUCCESS) {
        print_error("render()", "wait for fence failed! Code:", result);
        curFrame = (curFrame + 1) % maxImageCount;
        return;
    }
    // 请求下一张图像，确保可用
    acquire_next_image(&image_index, semaphores[curFrame], VK_NULL_HANDLE);
    auto& curBuf = cmdBuffers[curFrame];
    result = vkResetCommandBuffer(VkCommandBuffer(curBuf), 0);
    if (result != VK_SUCCESS) {
        print_error("render()", "vkResetCommandBuffer() failed! Code:", result);
    }
    curBuf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}
void RenderContext::beginRenderPass() {}
void RenderContext::endRenderPass() {}
void RenderContext::endRender() {
    auto& curBuf = cmdBuffers[curFrame];
    if (ownership_transfer) {
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
        .signalSemaphoreCount = 1};
    VkFence fence;
    if (delayOwnershipTransfer) {
        submit_info.pSignalSemaphores =
            loop.semsRenderMainPassFinish[loop.curFrame].getPointer();
        fence = VK_NULL_HANDLE;
    } else {
        if (loop.ownership_transfer) {
            submit_info.pSignalSemaphores =
                loop.semsOwnershipIsTransfered[loop.curFrame].getPointer();
            fence = VK_NULL_HANDLE;
        } else {
            submit_info.pSignalSemaphores =
                loop.semsRenderFinish[loop.curFrame].getPointer();
            fence = VkFence(loop.fences[loop.curFrame]);
        }
    }
    VkResult result = vkQueueSubmit(CurContext().vulkanInfo.queue_graphics, 1,
                                    &submit_info, fence);
    if (result != VK_SUCCESS) {
        print_error("render()", "vkQueueSubmit() failed! Code:", result);
    }
}
void RenderContext::present() {}
}  // namespace BL
