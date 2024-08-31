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
        result = cmdPool_graphics.allocate_buffers(
            cmdBuffers.data(),
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
    semaphores.resize((pInit->renderPassCount + 1)*MAX_FLIGHT_COUNT);
    for (auto& it : semaphores)
        it.create();
    return VK_SUCCESS;
CREATE_FAILED:
    destroy();
    print_error("RenderContext", "Initialize failed! Code:", result);
    return result;
}
void RenderContext::destroy() {}
void RenderContext::beginRender() {}
void RenderContext::beginRenderPass() {}
void RenderContext::endRenderPass() {}
void RenderContext::endRender() {}
void RenderContext::present() {}
}  // namespace BL
