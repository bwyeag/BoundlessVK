#include <render.hpp>
namespace BL {
const VkExtent2D& window_size =
    context.vulkanInfo.swapchainCreateInfo.imageExtent;
RenderPassPack* pRenderpass_pack;
RenderContext::RenderContext(uint32_t c)
    : maxFlightCount(c),
      curFrame(0),
      cmdPool(context.vulkanInfo.queueFamilyIndex_graphics,
              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
      semsImageAvaliable(c),
      semsRenderFinish(c) {
    cmdBufs.resize(c);
    VkResult result = cmdPool.allocate_buffers(cmdBufs.data(), c);
    fences.reserve(c);
    for (uint32_t i = 0; i < c; i++) {
        fences.emplace_back(VK_FENCE_CREATE_SIGNALED_BIT);
    }
}
void initRenderLoop() {
    context.renderInfo = new RenderContext(DEFAULT_MAX_FLIGHT_COUNT);
    pRenderpass_pack = new RenderPassPack();
}
void terminateRenderLoop() {
    delete pRenderpass_pack;
    delete context.renderInfo;
}
void renderLoop() {
    auto& vkInfo = context.vulkanInfo;
    auto& device = vkInfo.device;
    auto& swapchain = vkInfo.swapchain;
    auto& swapchainInfo = vkInfo.swapchainCreateInfo;
    RenderContext& loop = *context.renderInfo;
    if (swapchainInfo.oldSwapchain && swapchainInfo.oldSwapchain != swapchain) {
        vkDestroySwapchainKHR(device, swapchainInfo.oldSwapchain, nullptr);
        swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    }
    // 等待当前帧的栅栏，确保先前的命令已完成执行
    VkResult result = loop.fences[loop.curFrame].wait_and_reset();
    if (result != VK_SUCCESS) {
        print_error("renderLoop()", "wait for fence failed! Code:", result);
        loop.curFrame = (loop.curFrame + 1) % loop.maxFlightCount;
        return;
    }
    // 请求下一张图像，确保可用
    uint32_t image_index;
    VkExtent2D& t = context.vulkanInfo.swapchainCreateInfo.imageExtent;
    while (result =
               vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                     loop.semsImageAvaliable[loop.curFrame],
                                     VK_NULL_HANDLE, &image_index))
        switch (result) {
            case VK_SUBOPTIMAL_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                recreateSwapchain();
                print_log("RenderSize", "New size:", t.width, t.height);
                break;
            default:
                print_error(
                    "renderLoop()",
                    "wait for image in swapchain failed! Code:", result);
                return;
        }
    auto& curBuf = loop.cmdBufs[loop.curFrame];
    result = vkResetCommandBuffer((VkCommandBuffer)curBuf, 0);
    if (result != VK_SUCCESS) {
        print_error("renderLoop()",
                    "vkResetCommandBuffer() failed! Code:", result);
    }
    VkClearValue clearColor = {.color = {1.f, 0.f, 0.f, 1.f}};  // 红色
    curBuf.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    /*
     * do something
     */
    pRenderpass_pack->renderPass.cmd_begin(
        curBuf, pRenderpass_pack->framebuffers[loop.curFrame],
        {{}, window_size}, &clearColor, 1);
    /*渲染命令，待填充*/
    pRenderpass_pack->renderPass.cmd_end(curBuf);

    curBuf.end();
    // 发送渲染命令
    VkPipelineStageFlags flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = curBuf.getPointer();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores =
        loop.semsImageAvaliable[loop.curFrame].getPointer();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores =
        loop.semsRenderFinish[loop.curFrame].getPointer();
    submit_info.pWaitDstStageMask = &flag;
    result = vkQueueSubmit(context.vulkanInfo.queue_graphics, 1, &submit_info,
                           loop.fences[loop.curFrame]);
    if (result != VK_SUCCESS) {
        print_error("renderLoop()", "vkQueueSubmit() failed! Code:", result);
    }
    // 发送呈现命令
    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &context.vulkanInfo.swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores =
        loop.semsRenderFinish[loop.curFrame].getPointer();

    result =
        vkQueuePresentKHR(context.vulkanInfo.queue_presentation, &present_info);
    switch (result) {
        case VK_SUCCESS:
            break;
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
            recreateSwapchain();
            break;
        default:
            print_error("renderLoop()","Failed to queue the image for presentation! Code:",int32_t(result));
            break;
    }
    // 切换到下一帧
    loop.curFrame = (loop.curFrame + 1) % loop.maxFlightCount;
}
RenderPassPack::RenderPassPack() {
    VkAttachmentDescription attachmentDescription = {
        .format = context.vulkanInfo.swapchainCreateInfo.imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    VkAttachmentReference attachmentReference = {
        0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReference};
    VkSubpassDependency subpassDependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .attachmentCount = 1,
        .pAttachments = &attachmentDescription,
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency};
    renderPass.create(renderPassCreateInfo);

    auto CreateFramebuffers = [this] {
        framebuffers.resize(context.getSwapChainImageCount());
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .renderPass = renderPass,
            .attachmentCount = 1,
            .width = window_size.width,
            .height = window_size.height,
            .layers = 1};
        VkImageView attachment;
        for (size_t i = 0; i < context.getSwapChainImageCount(); i++) {
            attachment = context.vulkanInfo.swapchainImageViews[i];
            framebufferCreateInfo.pAttachments = &attachment;
            framebuffers[i].create(framebufferCreateInfo);
        }
    };
    auto DestroyFramebuffers = [this] { framebuffers.clear(); };
    CreateFramebuffers();

    callback_c_id = addCallback_CreateSwapchain(CreateFramebuffers);
    callback_d_id = addCallback_DestroySwapchain(DestroyFramebuffers);
}
RenderPassPack::~RenderPassPack() {
    removeCallback_CreateSwapchain(callback_c_id);
    removeCallback_DestroySwapchain(callback_d_id);
}
}  // namespace BL
