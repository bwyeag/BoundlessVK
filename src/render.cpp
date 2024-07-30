#include <render.hpp>
namespace BL {
const VkExtent2D& window_size =
    context.vulkanInfo.swapchainCreateInfo.imageExtent;
RenderPassPack* pRenderpass_pack;
RenderPipeline* pRenderPipeline;
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
    pRenderPipeline = new RenderPipeline();
}
void terminateRenderLoop() {
    delete pRenderPipeline;
    delete pRenderpass_pack;
    delete context.renderInfo;
}
VkResult createRenderContext() {
    context.renderContext = new RenderContextPlus();
    auto& info = context.vulkanInfo;
    auto& render_context = *context.renderContext;
    VkResult result;
    if (info.queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED) {
        result = render_context.cmdPool_graphics.create(
            info.queueFamilyIndex_graphics,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result) {
            destroyRenderContext();
            return result;
        }
        result = render_context.cmdPool_graphics.allocate_buffer(
            &render_context.cmdBuffer_transfer);
        if (result) {
            destroyRenderContext();
            return result;
        }
    }
    if (info.queue_compute != VK_QUEUE_FAMILY_IGNORED) {
        result = render_context.cmdPool_compute.create(
            info.queue_compute,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result) {
            destroyRenderContext();
            return result;
        }
    }
    if (info.queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
        info.queueFamilyIndex_presentation != info.queueFamilyIndex_graphics &&
        info.swapchainCreateInfo.imageSharingMode ==
            VK_SHARING_MODE_EXCLUSIVE) {
        result = render_context.cmdPool_presentation.create(
            info.queueFamilyIndex_presentation,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (result) {
            destroyRenderContext();
            return result;
        }
        result = render_context.cmdPool_presentation.allocate_buffer(
            &render_context.cmdBuffer_presentation);
        if (result) {
            destroyRenderContext();
            return result;
        }
    }
    return VK_SUCCESS;
}
VkResult execute_cmdBuffer_graphics_wait(VkCommandBuffer commandBuffer) {
    Fence fence;
    VkResult result;
    result = execute_cmdBuffer_graphics(commandBuffer, fence);
    if (result)
        return result;
    else
        return fence.wait();
}
VkResult execute_cmdBuffer_graphics(VkCommandBuffer commandBuffer,
                                    Fence& fence) {
    VkSubmitInfo submitInfo = {.commandBufferCount = 1,
                               .pCommandBuffers = &commandBuffer};
    VkResult result =
        vkQueueSubmit(context.vulkanInfo.queue_graphics, 1, &submitInfo, fence);
    if (result) {
        print_error("execute_cmdBuffer_graphics",
                    "submit failed! Code:", int32_t(result));
        return result;
    }
    return VK_SUCCESS;
}
// void CmdTransferImageOwnership(VkCommandBuffer commandBuffer) const {
//     VkImageMemoryBarrier imageMemoryBarrier_g2p = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//         .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//         .dstAccessMask = 0,
//         .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//         .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//         .srcQueueFamilyIndex = queueFamilyIndex_graphics,
//         .dstQueueFamilyIndex = queueFamilyIndex_presentation,
//         .image = swapchainImages[currentImageIndex],
//         .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
//     };
//     vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
//         0, nullptr, 0, nullptr, 1, &imageMemoryBarrier_g2p);
// }
// VkResult AcquireImageOwnership_Presentation(
//     VkSemaphore semaphore_renderingIsOver,
//     VkSemaphore semaphore_ownershipIsTransfered,
//     VkFence fence = VK_NULL_HANDLE) const {
//     if (VkResult result = commandBuffer_presentation.Begin(
//             VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
//         return result;
//     graphicsBase::Base().CmdTransferImageOwnership(commandBuffer_presentation);
//     if (VkResult result = commandBuffer_presentation.End())
//         return result;
//     return graphicsBase::Base().SubmitCommandBuffer_Presentation(
//         commandBuffer_presentation, semaphore_renderingIsOver,
//         semaphore_ownershipIsTransfered, fence);
// }
void destroyRenderContext() {
    auto& render_context = *context.renderContext;
    render_context.cmdPool_presentation.~CommandPool();
    render_context.cmdPool_graphics.~CommandPool();
    render_context.cmdPool_compute.~CommandPool();
    delete context.renderContext;
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

    pRenderpass_pack->renderPass.cmd_begin(
        curBuf, pRenderpass_pack->framebuffers[loop.curFrame],
        {{}, window_size}, &clearColor, 1);
    vkCmdBindPipeline(curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VkPipeline(pRenderPipeline->renderPipeline));
    vkCmdDraw(curBuf, 3, 1, 0, 0);
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
            print_error("renderLoop()",
                        "Failed to queue the image for presentation! Code:",
                        int32_t(result));
            break;
    }
    // 切换到下一帧
    loop.curFrame = (loop.curFrame + 1) % loop.maxFlightCount;
}
void RenderPassPack::create() {
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
void RenderPipeline::create() {
    shader.create(
        "D:\\c++programs\\BoundlessVK\\BoundlessVK\\shader\\shader.shader");
    auto Create = [this] {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layout.create(pipelineLayoutCreateInfo);
        PipelineCreateInfosPack pack;
        pack.createInfo.layout = VkPipelineLayout(layout);
        pack.createInfo.renderPass = VkRenderPass(pRenderpass_pack->renderPass);
        pack.inputAssemblyStateCi.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pack.viewports.emplace_back(0.0f, 0.0f, float(window_size.width),
                                    float(window_size.height), 0.f, 1.f);
        pack.scissors.emplace_back(VkOffset2D{}, window_size);
        pack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pack.colorBlendAttachmentStates.push_back(
            VkPipelineColorBlendAttachmentState{.colorWriteMask = 0b1111});
        pack.update_all_arrays();
        pack.createInfo.stageCount = shader.getStages().size();
        pack.createInfo.pStages = shader.getStages().data();
        renderPipeline.create(pack);
    };
    auto Destroy = [this] { renderPipeline.~Pipeline(); };
    callback_c_id = addCallback_CreateSwapchain(Create);
    callback_d_id = addCallback_DestroySwapchain(Destroy);
    Create();
}
}  // namespace BL
