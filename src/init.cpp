#include "BL/init.hpp"
namespace BL {
Context context;
void setWindowInit(int w, int h) {
    context.windowInfo.width = w;
    context.windowInfo.height = h;
}
void setWindowTitle(const char* newTitle) {
    glfwSetWindowTitle(context.windowInfo.pWindow, newTitle);
}
bool initWindow(const char* title, bool fullScreen, bool isResizable) {
    if (!glfwInit()) {
        std::cout << "[Error]GLFW init failed!\n";
        return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    context.windowInfo.pMonitor = glfwGetPrimaryMonitor();
    if (fullScreen) {
        const GLFWvidmode* pMode = glfwGetVideoMode(context.windowInfo.pMonitor);
        context.windowInfo.width = pMode->width;
        context.windowInfo.height = pMode->height;
        context.windowInfo.pWindow = glfwCreateWindow(
            pMode->width, pMode->height, title,
            context.windowInfo.pMonitor, nullptr);
    } else {
        context.windowInfo.pWindow = glfwCreateWindow(
            context.windowInfo.width, context.windowInfo.height,
            title, nullptr, nullptr);
    }
    if (!context.windowInfo.pWindow) {
        std::cout << "[Error]GLFW window create failed!\n";
        glfwTerminate();
        return false;
    }
    return true;
}
bool terminateWindow() {
    glfwDestroyWindow(context.windowInfo.pWindow);
    glfwTerminate();
    return true;
}
}  // namespace BL