#include <BL/init.hpp>
#include <iostream>
int main() {
    BL::setWindowInit(800, 600, true);
    if (!BL::initWindow("Boundless", false, false)||!BL::initVulkan()) {
        return -1;
    }
    while (!BL::checkWindowClose()) {
        glfwPollEvents();
        BL::calcFps();
    }

    BL::terminateVulkan();
    BL::terminateWindow();
    return 0;
}