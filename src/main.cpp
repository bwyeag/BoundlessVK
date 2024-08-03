#include <BL/init.hpp>
#include <BL/render.hpp>
#include <iostream>
#include <cmath>

BL::Vertex_2d vertices[] = {
    {{.0f, -.5f}, {1, 0, 0, 1}},  // 红色
    {{-.5f, .5f}, {0, 1, 0, 1}},  // 绿色
    {{.5f, .5f}, {0, 0, 1, 1}}    // 蓝色
};
BL::vec2f uniform_data[] = {{.0f, .0f}, {}, {-.5f, .0f}, {}, {.5f, .0f}};
void render_funct_simple1(BL::CommandBuffer& curBuf,
                          BL::RenderDataPack& packet,
                          uint32_t image_index) {
    uint32_t curFrame = BL::render_context.curFrame;
    VkClearValue clearColor = {.color = {0.1f, 0.1f, 0.1f, 1.0f}};
    packet.pRenderPass->renderPass.cmd_begin(
        curBuf, packet.pRenderPass->framebuffers[image_index],
        {{}, BL::context.vulkanInfo.swapchainCreateInfo.imageExtent},
        &clearColor, 1);
    vkCmdBindPipeline(curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VkPipeline(packet.pRenderPipline->renderPipeline));
    auto pipeline =
        dynamic_cast<BL::RenderPipeline_simple1*>(packet.pRenderPipline);
    vkCmdBindDescriptorSets(
        curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, 0, 1,
        pipeline->descriptorSets[curFrame].getPointer(), 0, nullptr);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(curBuf, 0, 1, packet.pVertexData->getPointer(),
                           &offset);
    vkCmdDraw(curBuf, 3, 3, 0, 0);
    packet.pRenderPass->renderPass.cmd_end(curBuf);
}
constexpr double MATH_PI = 3.1415926535;
int main() {
    BL::setWindowInit(800, 600, true);
    if (!BL::initWindow("Boundless", false, true) || !BL::initVulkan()) {
        return -1;
    }
    BL::initVulkanRenderer();
    double t0 = glfwGetTime();
    {
        //---------------------------------------------
        BL::Shader shader(
            ".\\shader.shader");
        BL::RenderPassPack_simple1 renderpass_pack;
        BL::RenderPipeline_simple1 renderPipeline;

        VkDeviceSize uniformAlignment =
            BL::context.vulkanInfo.phyDeviceProperties.limits
                .minUniformBufferOffsetAlignment;
        renderPipeline.uniformSize = sizeof(uniform_data);
        VkDeviceSize uniformBlockAlignment =
            uniformAlignment *
            std::ceil(sizeof(uniform_data) / double(uniformAlignment));
        renderPipeline.uniformBuffer.allocate(
            uniformBlockAlignment * BL::MAX_FLIGHT_NUM, 0,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            VMA_MEMORY_USAGE_AUTO);
        uint8_t* ptr = (uint8_t*)renderPipeline.uniformBuffer.map_data();
        for (uint32_t i = 0; i < BL::MAX_FLIGHT_NUM; i++) {
            memcpy(ptr + uniformBlockAlignment * i, uniform_data,
                   sizeof(uniform_data));
        }
        renderPipeline.uniformBuffer.flush_data();
        renderPipeline.uniformBuffer.unmap_data();

        renderPipeline.create(&shader, &renderpass_pack);

        BL::Buffer vertex_buffer(
            sizeof(vertices), 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO);
        vertex_buffer.transfer_data(&vertices, sizeof(vertices));
        BL::RenderDataPack renderPack = {&renderpass_pack, &renderPipeline,
                                         &vertex_buffer, render_funct_simple1};
        while (!BL::checkWindowClose()) {
            while (glfwGetWindowAttrib(BL::context.windowInfo.pWindow,
                                       GLFW_ICONIFIED))
                glfwWaitEvents();
            //------------------------------------------------------
            double t2 = glfwGetTime() - t0;
            uniform_data[0] = {0.8*std::cos(t2),0.8*std::sin(t2)};
            uniform_data[2] = {0.7*std::cos(t2+2*MATH_PI/3),0.7*std::sin(t2+2*MATH_PI/3)};
            uniform_data[4] = {0.6*std::cos(t2+4*MATH_PI/3),0.6*std::sin(t2+4*MATH_PI/3)};
            uint8_t* ptr = (uint8_t*)renderPipeline.uniformBuffer.map_data();
            for (uint32_t i = 0; i < BL::MAX_FLIGHT_NUM; i++) {
                memcpy(ptr + uniformBlockAlignment * i, uniform_data,
                    sizeof(uniform_data));
            }
            renderPipeline.uniformBuffer.flush_data();
            renderPipeline.uniformBuffer.unmap_data();
            //------------------------------------------------------
            BL::render(renderPack);
            glfwPollEvents();
            BL::calcFps();
        }
        BL::waitAll();
    }
    BL::terminateVulkanRenderer();
    BL::terminateVulkan();
    BL::terminateWindow();
    system("pause");
    return 0;
}