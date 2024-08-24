#include <BL/init.hpp>
#include <BL/mesh.hpp>
#include <BL/render.hpp>
#include <BL/camera.hpp>
#define BL_PBR_SIMPLE_MAX_LIGHT_NUM 4
#define BL_PBR_SIMPLE_MAX_INSTANCE_NUM 100
#include <BL/render_types.hpp>
#include <cmath>
#include <iostream>

using namespace BL;
Camera_debug camera;
float deltaTime = 0.1f;
float lastTime = 0.0f;
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.process_keyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.process_keyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.process_keyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.process_keyboard(RIGHT, deltaTime);
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    static bool firstMouse = true;
    static float lastX = 0.0f;
    static float lastY = 0.0f;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    camera.process_mouse_movement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.process_mouse_scroll(static_cast<float>(yoffset));
}
int main() {
    setWindowInit(800, 600, true);
    if (!initWindow("Boundless", false, true) || !initVulkan()) {
        return -1;
    }
    glfwSetCursorPosCallback(context.windowInfo.pWindow, mouse_callback);
    glfwSetInputMode(context.windowInfo.pWindow, GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(context.windowInfo.pWindow, scroll_callback);
    initVulkanRenderer();
    initDepthAttachment();
    {
        Uniform_LightUniform uniform_light_uniform;
        uniform_light_uniform.lightCount = 4;
        uniform_light_uniform.light[0] = {
            .lightPosition = vec3f(10.0f, 27.5f, 27.5f),
            .lightColor = vec3f(300.0f, 300.0f, 300.0f)};
        uniform_light_uniform.light[1] = {
            .lightPosition = vec3f(10.0f, 27.5f, 17.5f),
            .lightColor = vec3f(300.0f, 300.0f, 300.0f)};
        uniform_light_uniform.light[2] = {
            .lightPosition = vec3f(10.0f, 17.5f, 17.5f),
            .lightColor = vec3f(300.0f, 300.0f, 300.0f)};
        uniform_light_uniform.light[3] = {
            .lightPosition = vec3f(10.0f, 17.5f, 27.5f),
            .lightColor = vec3f(300.0f, 300.0f, 300.0f)};
        Uniform_SenceUniform uniform_sence_uniform;
        ModelTransform modelTrans;
        modelTrans.rotate = rotate(vec3f(MATH_PI / 4, MATH_PI / 3, 0.0f));
        modelTrans.scale = vec3f(1.5f, 1.5f, 1.5f);
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                modelTrans.isEdited = true;
                modelTrans.position = vec3f(0, 5.0f * j, 5.0f * i);
                uniform_sence_uniform.instance[i * 10 + j] = {
                    .matrixModel = modelTrans.get_matrix(),
                    .albedo = vec3f(0.8f, 0.0f, 0.0f),
                    .metallic = 0.1f * i,
                    .roughness = 0.1f * j,
                    .ao = 1.0f};
                uniform_sence_uniform.instance[i * 10 + j]
                    .matrixNormal.block<3, 3>(0, 0) =
                    uniform_sence_uniform.instance[10 * i + j]
                        .matrixModel.block<3, 3>(0, 0)
                        .inverse()
                        .transpose();
            }
        }
        camera.worldUp = vec3f(0.0f,0.0f,1.0f);
        camera.worldForward = vec3f(0.0f,1.0f,0.0f);
        camera.worldRight = vec3f(1.0f,0.0f,0.0f);
        camera.yaw = 0.0f;
        camera.pitch = 0.0f;
        camera.mouseSensitivity = 0.004f;
        camera.moveSpeed = 3.0f;
        camera.transform.isViewEdited = true;
        camera.position() = vec3f(0.0f,0.0f,0.0f);
        camera.forward() = vec3f(1.0f,0.0f,0.0f);
        camera.up() = vec3f(0.0f,0.0f,1.0f);
        camera.transform.isProjEdited = true;
        camera.transform.isFrustum = true;
        camera.transform.zNear = 0.1f;
        camera.transform.zFar = 50.0f;
        camera.fov() = radians(45.0f);
        bool changedFlag;
        uniform_sence_uniform.cameraPosition =
            vec3f(camera.position().x(), camera.position().y(),
                  camera.position().z());
        uniform_sence_uniform.matrixCamera =
            camera.get_proj_matrix(changedFlag) *
            camera.get_view_matrix(changedFlag);

        RenderDataPack_PBR_3d_simple renderPack;
        renderPack.pRenderFunction = render_funct_PBR_3d_simple;

        Mesh mesh("h_huan.mesh");
        renderPack.meshData = &mesh;

        uint32_t specArray[2]{BL_PBR_SIMPLE_MAX_LIGHT_NUM,
                              BL_PBR_SIMPLE_MAX_INSTANCE_NUM};
        VkSpecializationMapEntry mapEntry[2]{
            {.constantID = 0, .offset = 0, .size = 4},
            {.constantID = 1, .offset = 4, .size = 4},
        };
        VkSpecializationInfo spec_info = {.mapEntryCount = 2,
                                          .pMapEntries = mapEntry,
                                          .dataSize = sizeof(specArray),
                                          .pData = specArray};
        Shader shader("pbr.shader", &spec_info);

        RenderPassPack_PBR_3d_simple renderpass_pack;
        RenderPipeline_PBR_3d_simple renderPipeline;
        renderPack.pRenderPass = &renderpass_pack;
        renderPack.pRenderPipeline = &renderPipeline;

        renderPipeline.uniformBuf_LightUniform.create(
            sizeof(uniform_light_uniform));
        renderPipeline.uniformBuf_SenceData.create(
            sizeof(uniform_sence_uniform));

        renderPipeline.uniformBuf_LightUniform.transfer_data(
            &uniform_light_uniform);
        renderPipeline.uniformBuf_SenceData.transfer_data(
            &uniform_sence_uniform);

        renderPipeline.create(&shader, &renderpass_pack, &mesh);

        while (!checkWindowClose()) {
            float currentTime = static_cast<float>(glfwGetTime());
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            while (
                glfwGetWindowAttrib(context.windowInfo.pWindow, GLFW_ICONIFIED))
                glfwWaitEvents();

            uniform_sence_uniform.cameraPosition = camera.position();
            uniform_sence_uniform.matrixCamera =
                camera.get_proj_matrix(changedFlag) *
                camera.get_view_matrix(changedFlag);
            std::cout << camera.yaw << ',' << camera.pitch << '\n';
            if (changedFlag) {
                renderPipeline.uniformBuf_SenceData.transfer_data(
                    &uniform_sence_uniform);
                changedFlag = false;
            }
            render(renderPack);
            glfwPollEvents();
            process_input(context.windowInfo.pWindow);
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