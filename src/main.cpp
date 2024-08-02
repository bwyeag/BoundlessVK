#include <BL/init.hpp>
#include <BL/render.hpp>
#include <iostream>

BL::Vertex_2d vertices[] = {
    {{.0f, -.5f}, {1, 0, 0, 1}},  // 红色
    {{-.5f, .5f}, {0, 1, 0, 1}},  // 绿色
    {{.5f, .5f}, {0, 0, 1, 1}}    // 蓝色
};

int main() {
    BL::setWindowInit(800, 600, true);
    if (!BL::initWindow("Boundless", false, true) || !BL::initVulkan()) {
        return -1;
    }
    BL::initVulkanRenderer();
    {
        BL::Shader shader(
            "D:\\c++programs\\BoundlessVK\\BoundlessVK\\shader\\shader.shader");
        BL::RenderPassPack_simple1 renderpass_pack;
        BL::RenderPipeline_simple1 renderPipeline(&shader, &renderpass_pack);
        BL::Buffer vertex_buffer(
            sizeof(vertices), 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO);
        vertex_buffer.transfer_data(&vertices, sizeof(vertices));
        BL::RenderDataPack renderPack = {&renderpass_pack, &renderPipeline,
                                         &vertex_buffer};
        while (!BL::checkWindowClose()) {
            while (glfwGetWindowAttrib(BL::context.windowInfo.pWindow,
                                       GLFW_ICONIFIED))
                glfwWaitEvents();
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