#include <BL/camera.hpp>
#include <BL/init.hpp>
#include <BL/mesh.hpp>
#include <BL/render.hpp>
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
    if (!camera.isLocked()) {
        using CMV = CameraMovement;
        CMV key = NONE_MOVEMENT;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            key = CMV(key | FORWARD);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            key = CMV(key | BACKWARD);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            key = CMV(key | LEFT);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            key = CMV(key | RIGHT);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            key = CMV(key | ROTATE_LEFT);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            key = CMV(key | ROTATE_RIGHT);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            key = CMV(key | FORWARD_UP);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            key = CMV(key | FORWARD_DOWN);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            key = CMV(key | FORWARD_LEFT);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            key = CMV(key | FORWARD_RIGHT);
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            key = CMV(key | ZOOM_UP);
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            key = CMV(key | ZOOM_DOWN);
        camera.process_keyboard(key, deltaTime);
    }
}
void keyboard_callback(GLFWwindow* window,
                       int key,
                       int scancode,
                       int action,
                       int mods) {
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        camera.process_keyboard(LOCK, deltaTime);
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
    float yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;
    camera.process_mouse_movement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.process_mouse_scroll(static_cast<float>(yoffset));
}
int main() {
    system("chcp 65001");
    setWindowInit(800, 600, true);
    if (!initWindow("Boundless", false, true) || !initVulkan()) {
        return -1;
    }
    glfwSetCursorPosCallback(context.windowInfo.pWindow, mouse_callback);
    glfwSetKeyCallback(context.windowInfo.pWindow,keyboard_callback);
    glfwSetScrollCallback(context.windowInfo.pWindow, scroll_callback);
    glfwSetInputMode(context.windowInfo.pWindow, GLFW_CURSOR,
                     GLFW_CURSOR_NORMAL);
    initVulkanRenderer();
    initDepthAttachment();
    {
        print_log("使用说明","按L键解锁/加锁摄像机，WASD移动，QE滚转，TY缩小/放大,箭头上下左右/鼠标移动视角朝向");
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
        camera.set_zoom_range(0.1f, MATH_PI / 4.0f);
        camera.set_zoom_sensitivity(0.08f);
        camera.set_key_sensitivity(1.0f);
        camera.set_mouse_sensitivity(0.001f);
        camera.set_move_velocity(3.0f);
        camera.Position() = vec3f(0.0f, 0.0f, 0.0f);
        camera.Forward() = vec3f(1.0f, 0.0f, 0.0f);
        camera.Up() = vec3f(0.0f, 0.0f, 1.0f);
        camera.NearDist() = 0.1f;
        camera.FarDist() = 50.0f;
        camera.Fov() = MATH_PI / 4.0f;
        camera.set_proj_type(FRUSTUM);
        bool changedFlag;
        uniform_sence_uniform.cameraPosition = camera.Position();
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

            uniform_sence_uniform.cameraPosition = camera.Position();
            uniform_sence_uniform.matrixCamera =
                camera.get_proj_matrix(changedFlag) *
                camera.get_view_matrix(changedFlag);
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