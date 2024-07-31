#include <BL/init.hpp>
#include <BL/render.hpp>
#include <iostream>
int main() {
    BL::setWindowInit(800, 600, true);
    if (!BL::initWindow("Boundless", false, true)||!BL::initVulkan()) {
        return -1;
    }
    BL::initVulkanRenderer();
    while (!BL::checkWindowClose()) {
        while (glfwGetWindowAttrib(BL::context.windowInfo.pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();
        BL::render();
        glfwPollEvents();
        BL::calcFps();
    }
    BL::waitAll();
    BL::terminateVulkanRenderer();
    BL::terminateVulkan();
    BL::terminateWindow();
    system("pause");
    return 0;
}