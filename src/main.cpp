#include <BL/init.hpp>
#include <BL/render.hpp>
#define BL_USE_RENDER_TYPE_3D_TRANSFORM
#include <BL/mesh.hpp>
#include <BL/render_types.hpp>
#include <cmath>
#include <iostream>

using namespace BL;
constexpr double MATH_PI = 3.1415926535;
int main() {
    setWindowInit(800, 600, true);
    if (!initWindow("Boundless", false, true) || !initVulkan()) {
        return -1;
    }
    initVulkanRenderer();
    initDepthAttachment();
    double t0 = glfwGetTime();
    {
        UniformData_3d_trans_simple1 uniform_data;
        CameraTransform camera;
        camera.zFar = 25.0f;
        camera.zNear = 0.8f;
        camera.forward = vec3f(-1.0f, -1.0f, -1.0f);
        camera.position = vec3f(7.0f, 7.0f, 7.0f);
        camera.up = vec3f(0.0f, 0.0f, 1.0f);
        camera.up = camera.up.cross(camera.forward);
        camera.up = camera.forward.cross(camera.up);
        camera.fov = MATH_PI / 2;
        camera.isFrustum = true;
        bool changedFlag = false;
        uniform_data.cameraMat = camera.get_proj_matrix(changedFlag) *
                                 camera.get_view_matrix(changedFlag);

        RenderDataPack_3d_trans_simple1 renderPack;
        renderPack.pRenderFunction = render_funct_3d_trans_simple1;
        ModelTransform modelTrans1;
        modelTrans1.isEdited = true;
        modelTrans1.position = vec3f(0.0f, 0.0f, 0.0f);
        modelTrans1.rotate = quatf::Identity();
        modelTrans1.scale = vec3f(4.0f, 4.0f, 4.0f);
        uniform_data.model[0] = modelTrans1.get_matrix();
        ModelTransform modelTrans2;
        modelTrans2.isEdited = true;
        modelTrans2.position = vec3f(6.0f, 0.0f, 0.0f);
        modelTrans2.rotate = rotate(vec3f(MATH_PI/2,0.0f,0.0f));
        modelTrans2.scale = vec3f(2.0f, 2.0f, 2.0f);
        uniform_data.model[1] = modelTrans2.get_matrix();
        ModelTransform modelTrans3;
        modelTrans3.isEdited = true;
        modelTrans3.position = vec3f(0.0f, 3.0f, 0.0f);
        modelTrans3.rotate = rotate(vec3f(-MATH_PI/2,0.0f,0.0f));
        modelTrans3.scale = vec3f(2.0f, 2.0f, 2.0f);
        uniform_data.model[2] = modelTrans3.get_matrix();


        Mesh mesh(
            "D:\\c++programs\\BoundlessVK\\BoundlessVK\\utility_program\\h_"
            "huan.mesh");
        renderPack.meshData = &mesh;
        Shader shader(
            "D:\\c++programs\\BoundlessVK\\BoundlessVK\\shader\\shader."
            "shader");
        RenderPassPack_3d_trans_simple1 renderpass_pack;
        RenderPipeline_3d_trans_simple1 renderPipeline;
        renderPack.pRenderPass = &renderpass_pack;
        renderPack.pRenderPipeline = &renderPipeline;

        renderPipeline.uniformBuffer.create(sizeof(uniform_data));
        renderPipeline.uniformBuffer.transfer_data(&uniform_data);

        renderPipeline.create(&shader, &renderpass_pack, &renderPack);

        while (!checkWindowClose()) {
            while (
                glfwGetWindowAttrib(context.windowInfo.pWindow, GLFW_ICONIFIED))
                glfwWaitEvents();
            double t2 = glfwGetTime();
            modelTrans1.isEdited = true;
            modelTrans1.rotate = rotate(vec3f(static_cast<float>(t2-t0),0.0f,0.0f));
            uniform_data.model[0] = modelTrans1.get_matrix();
            uniform_data.cameraMat = camera.get_proj_matrix(changedFlag) *
                                     camera.get_view_matrix(changedFlag);
            renderPipeline.uniformBuffer.transfer_data(&uniform_data);
            render(renderPack);
            glfwPollEvents();
            calcFps();
        }
        waitAll();
    }
    destroyDepthAttachment();
    terminateVulkanRenderer();
    terminateVulkan();
    terminateWindow();
    system("pause");
    return 0;
}