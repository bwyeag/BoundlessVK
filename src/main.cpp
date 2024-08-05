#include <BL/init.hpp>
#include <BL/render.hpp>
#include <BL/mesh.hpp>
#include <cmath>
#include <iostream>

using namespace BL;
Vertex_2d vertices[] = {
    {{.0f, -.5f}, {1, 0, 0, 1}},  // 红色
    {{-.5f, .5f}, {0, 1, 0, 1}},  // 绿色
    {{.5f, .5f}, {0, 0, 1, 1}}    // 蓝色
};
struct UniformData {
    mat4f model[3];
    mat4f cameraMat;
};
UniformData uniform_data;
void render_funct_simple1(CommandBuffer& curBuf,
                          RenderDataPackBase& packBase,
                          uint32_t image_index) {
    RenderDataPack_simple1& packet = reinterpret_cast<RenderDataPack_simple1&>(packBase);
    uint32_t curFrame = render_context.curFrame;
    VkClearValue clearColor = {.color = {0.1f, 0.1f, 0.1f, 1.0f}};
    packet.pRenderPass->renderPass.cmd_begin(
        curBuf, packet.pRenderPass->framebuffers[image_index],
        {{}, context.vulkanInfo.swapchainCreateInfo.imageExtent},
        &clearColor, 1);
    vkCmdBindPipeline(curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      VkPipeline(packet.pRenderPipline->renderPipeline));
    vkCmdBindDescriptorSets(
        curBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.pRenderPipline->layout, 0, 1,
        packet.pRenderPipline->descriptorSets[curFrame].getPointer(), 0, nullptr);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(curBuf, 0, 1, packet.pVertexData->getPointer(),
                           &offset);
    vkCmdDraw(curBuf, 3, 3, 0, 0);
    packet.pRenderPass->renderPass.cmd_end(curBuf);
}
constexpr double MATH_PI = 3.1415926535;
int main() {
    setWindowInit(800, 600, true);
    if (!initWindow("Boundless", false, true) || !initVulkan()) {
        return -1;
    }
    initVulkanRenderer();
    double t0 = glfwGetTime();
    {
        ModelTransform obj1;
        ModelTransform obj2;
        ModelTransform obj3;
        obj1.position = vec3f(0.0f,0.0f,0.0f);
        obj2.position = vec3f(1.0f,-1.0f,0.0f);
        obj3.position = vec3f(-1.0f,1.0f,0.0f);
        obj1.rotate = quatf::Identity();
        obj2.rotate = quatf::Identity();
        obj3.rotate = quatf::Identity();
        obj1.scale = vec3f(0.5f,0.5f,0.5f);
        obj2.scale = vec3f(0.2f,0.2f,0.2f);
        obj3.scale = vec3f(1.0f,1.0f,1.0f);
        uniform_data.model[0] = obj1.get_matrix();
        uniform_data.model[1] = obj2.get_matrix();
        uniform_data.model[2] = obj3.get_matrix();

        CameraTransform camera;
        camera.zFar = 10.0f;
        camera.zNear = 0.1f;
        camera.forward = vec3f(-1.0f,-1.0f,-1.0f);
        camera.position = vec3f(2.0f,2.0f,2.0f);
        camera.up = vec3f(0.0f,0.0f,1.0f);
        camera.up = camera.up.cross(camera.forward);
        camera.up = camera.forward.cross(camera.up);
        camera.fov = MATH_PI/3;
        camera.isFrustum = true;
        uniform_data.cameraMat = camera.get_proj_matrix() * camera.get_view_matrix();

        for (size_t i = 0; i < 3; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                vec4f v(vertices[j].position.x(),vertices[j].position.y(),0.0f,1.0f);
                v = uniform_data.cameraMat * uniform_data.model[i] * v;
                v /= v.w();
                std::cout << "-----"<< i <<"\n";
                std::cout << v <<'\n';
            }
            
        }
        

        Shader shader("D:\\c++programs\\BoundlessVK\\BoundlessVK\\shader\\shader.shader");
        RenderPassPack_simple1 renderpass_pack;
        RenderPipeline_simple1 renderPipeline;

        renderPipeline.uniformBuffer.create(sizeof(uniform_data));
        renderPipeline.uniformBuffer.transfer_data(&uniform_data);

        renderPipeline.create(&shader, &renderpass_pack);

        Buffer vertex_buffer(
            sizeof(vertices), 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO);
        vertex_buffer.transfer_data(&vertices, sizeof(vertices));
        RenderDataPack_simple1 renderPack = {
            render_funct_simple1, &renderpass_pack, &renderPipeline,
            &vertex_buffer};
        while (!checkWindowClose()) {
            while (glfwGetWindowAttrib(context.windowInfo.pWindow,
                                       GLFW_ICONIFIED))
                glfwWaitEvents();
            render(renderPack);
            glfwPollEvents();
            calcFps();
        }
        waitAll();
    }
    terminateVulkanRenderer();
    terminateVulkan();
    terminateWindow();
    system("pause");
    return 0;
}