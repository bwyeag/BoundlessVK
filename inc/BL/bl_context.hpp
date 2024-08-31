#ifndef _BOUNDLESS_CONTEXT_CXX_FILE_
#define _BOUNDLESS_CONTEXT_CXX_FILE_
#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "log.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "callback.hpp"
#include "vk_mem_alloc.h"
namespace BL {
const uint32_t MAX_FLIGHT_COUNT = 3;
struct ContextInstanceInit {
    const char* pAppName = nullptr;
    uint32_t appVersion = 0u;
    uint32_t minApiVersion;
    bool isDebuging = false;
    const char** ppLayerNames = nullptr;
    uint32_t layerCount = 0u;
    const char** ppExtensionNames = nullptr;
    uint32_t extensionCount = 0u;
};
struct ContextDeviceInit {
    std::function<bool(VkPhysicalDevice)> isDeviceSuitable;
    uint32_t surfaceCount;
    VkSurfaceKHR* surface;
    VmaAllocatorCreateFlags vmaFlags;
    bool debug_print_deviceExtension;
};
struct Context {
    uint32_t vulkanApiVersion;
    VkInstance instance;
    VkPhysicalDevice phyDevice;
    VkDevice device;
    uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
    uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;
    uint32_t queueFamilyIndex_presentation = VK_QUEUE_FAMILY_IGNORED;
    VkQueue queue_graphics;
    VkQueue queue_compute;
    VkQueue queue_presentation;

    VkPhysicalDeviceProperties2 phyDeviceProperties;
    VkPhysicalDeviceVulkan11Properties phyDeviceVulkan11Properties;
    VkPhysicalDeviceVulkan12Properties phyDeviceVulkan12Properties;
    VkPhysicalDeviceVulkan13Properties phyDeviceVulkan13Properties;

    VkPhysicalDeviceMemoryProperties2 phyDeviceMemoryProperties;

    VkPhysicalDeviceFeatures2 phyDeviceFeatures;
    VkPhysicalDeviceVulkan11Features phyDeviceVulkan11Features;
    VkPhysicalDeviceVulkan12Features phyDeviceVulkan12Features;
    VkPhysicalDeviceVulkan13Features phyDeviceVulkan13Features;
#ifdef BL_DEBUG
    VkDebugUtilsMessengerEXT debugger;
#endif  // BL_DEBUG
    VmaAllocator allocator;

    Context() = default;
    Context(const Context&) = delete;
    Context(Context&& other) = delete;
    ~Context() {}

    bool createInstance(const ContextInstanceInit* pInit);
    bool createDevice(const ContextDeviceInit* pInit);
    void destroyDevice();
    void destroyInstance();
    void terminate() {
        destroyDevice();
        destroyInstance();
    }
    VkResult getApiVersion(uint32_t* apiVersion);

   private:
    std::vector<const char*> _getInstanceLayer(
        const ContextInstanceInit* pInit);
    std::vector<const char*> _getInstanceExtension(
        const ContextInstanceInit* pInit);
    VkResult _getPhysicalDevices(
        std::vector<VkPhysicalDevice>& avaliablePhyDevices);
    std::vector<const char*> _selectDeviceExtensions(
        const std::vector<VkExtensionProperties>& supported_ext);
    VkResult _getQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                             const ContextDeviceInit* pInit);
    VkResult _selectPhysicalDevice(const ContextDeviceInit* pInit);
    std::vector<VkExtensionProperties> _getDeviceExtensions(const ContextDeviceInit* pInit);
    VkResult _initVMA(const ContextDeviceInit* pInit);
#ifdef BL_DEBUG
    VkResult _createDebugMessenger();
#endif  // BL_DEBUG
};
extern Context _context;
inline Context& CurContext() {
    return _context;
}
struct WindowContextInit {
    const char* initTitle;
    uint32_t initWidth, initHeight;
    bool isFullScreen = false, isFpsShowed = true, isCursorVisable = true,
         isResizable = false;
    uint32_t maxWidth = GLFW_DONT_CARE, minWidth = GLFW_DONT_CARE;
    uint32_t maxHeight = GLFW_DONT_CARE, minHeight = GLFW_DONT_CARE;
};
struct WindowContextSwapchainInit {
    VkSwapchainCreateFlagsKHR flags = 0;
    bool isFrameRateLimited = true;
};
struct WindowContext {
    GLFWwindow* pWindow = nullptr;
    GLFWmonitor* pMonitor = nullptr;
    std::string title;
    uint32_t lastWidth, lastHeight;
    uint32_t lastPosX, lastPosY;
    uint32_t maxWidth, minWidth;
    uint32_t maxHeight, minHeight;
    double currentTime = 0.0, deltaTime = 0.0;
    bool isFullScreen, isFpsShowed;

    uint32_t deltaFrame = 0;
    std::stringstream stringBuilder;
    double lastTime = 0.0;  // FPS显示使用的时间

    CallbackListVoid callback_createSwapchain;
    CallbackListVoid callback_destroySwapchain;
    CallbackList<GLFWwindow*, double, double> callback_cursorPos;
    CallbackList<GLFWwindow*, int, int, int, int> callback_key;
    CallbackList<GLFWwindow*, double, double> callback_scroll;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

    WindowContext() = default;
    WindowContext(const WindowContextInit* pInit) { createWindow(pInit); }
    WindowContext(const WindowContext&) = delete;
    // WindowContext(WindowContext&& other);
    ~WindowContext() {}
    bool createWindow(const WindowContextInit* pInit);
    void update();
    void destroy();

    void setTitle(const char* newTitle);
    void setTitle(const std::string& newTitle);
    void setFullSrceen(bool flag);
    void setFpsShowed(bool flag);
    void setCursorVisable(bool flag);
    void setWindowSize(int offsetX, int offsetY, int width, int height);
    void setWindowSizeLimits(uint32_t maxWidth,
                             uint32_t minWidth,
                             uint32_t maxHeight,
                             uint32_t minHeight);
    void setWindowPos(uint32_t x,
                      uint32_t y) {
        glfwSetWindowPos(pWindow, x, y);
    }
    void setWindowSizeAspect();
    void setWindowSizeAspectClear() {
        glfwSetWindowAspectRatio(pWindow, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }
    void setWindowSizeAspect(uint32_t width, uint32_t height) {
        glfwSetWindowAspectRatio(pWindow, width, height);
    }
    void hideWindow() { glfwHideWindow(pWindow); }
    void showWindow() { glfwShowWindow(pWindow); }
    void maxsizeWindow() { glfwMaximizeWindow(pWindow); }
    void restoreWindow() { glfwRestoreWindow(pWindow); }
    inline bool isClosed() { return glfwWindowShouldClose(this->pWindow); }

    static bool initialize();
    static void terminate();

    VkResult recreateSwapchain();

    VkResult createSwapchain(const WindowContextSwapchainInit* pInit);
   private:
    VkResult _createSurface();
    VkResult _getSurfaceFormats(std::vector<VkSurfaceFormatKHR>& formats);
    VkResult _setSurfaceFormat(
        VkSurfaceFormatKHR surfaceFormat,
        std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);
    VkResult _createSwapChain_Internal();
};
}  // namespace BL_ext
#endif  //!_BOUNDLESS_CONTEXT_CXX_FILE_