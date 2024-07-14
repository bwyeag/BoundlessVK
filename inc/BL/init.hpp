#ifndef BOUNDLESS_INIT_FILE
#define BOUNDLESS_INIT_FILE
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
namespace BL {
struct WindowContext {
    GLFWwindow* pWindow;
    GLFWmonitor* pMonitor;
    int width,height;
};
struct Context {
    WindowContext windowInfo;
};
extern Context context;
void setWindowInit(int w, int h);
void setWindowTitle(const char* newTitle);
bool initWindow(const char* title, bool fullScreen, bool isResizable);
bool terminateWindow();
}  // namespace BL
#endif  //! BOUNDLESS_INIT_FILE