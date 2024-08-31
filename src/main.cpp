#include "bl_context.hpp"

using namespace BL;
int main() {
    Context& cxt = CurContext();
    WindowContext wcxt;
    {
        WindowContext::initialize();
        ContextInstanceInit instance_init = {
            .pAppName = "Test",
            .appVersion = VK_MAKE_API_VERSION(0, 1, 0, 1),
            .minApiVersion = VK_API_VERSION_1_0,
            .isDebuging = false};
        cxt.createInstance(&instance_init);
        WindowContextInit window_init = {.initTitle = "Boundless",
                                         .initWidth = 800,
                                         .initHeight = 600,
                                         .isFpsShowed = true};
        wcxt.createWindow(&window_init);
        VkSurfaceKHR surface[1]{wcxt.surface};
        ContextDeviceInit device_init = {.surfaceCount = 1,
                                         .surface = surface,
                                         .debug_print_deviceExtension = false
        };
        cxt.createDevice(&device_init);
        WindowContextSwapchainInit swapchain_init = {
            .flags = 0, .isFrameRateLimited = true};
        wcxt.createSwapchain(&swapchain_init);
    }
    {
        while (!wcxt.isClosed()) {
            glfwPollEvents();
            wcxt.update();
        }
    }
    wcxt.destroy();
    cxt.destroyDevice();
    cxt.destroyInstance();
    WindowContext::terminate();
    system("pause");
}