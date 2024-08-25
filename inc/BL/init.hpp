#ifndef BOUNDLESS_INIT_FILE
#define BOUNDLESS_INIT_FILE
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
#include <vulkan/vulkan.h>
#include "callback.hpp"
#include "vk_mem_alloc.h"
namespace BL {
const uint32_t API_VERSION = VK_API_VERSION_1_3;
struct WindowContext {
    GLFWwindow* pWindow;
    GLFWmonitor* pMonitor;
    std::string title;
    uint32_t width, height;
    bool isFullScreen = false;
    bool isFpsDisplayed_debug = false;
    double lastTime = 0.0;
    double currentTime = 0.0;
    double deltaTime = 0.0;
};
struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice phyDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkSwapchainCreateInfoKHR swapchainCreateInfo;
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
};
struct Context {
    WindowContext windowInfo;
    VulkanContext vulkanInfo;
    CallbackList<void()> callback_createSwapchain;
    CallbackList<void()> callback_destroySwapchain;
    CallbackList<void(GLFWwindow*, double, double)> callback_cursorPos;
    CallbackList<void(GLFWwindow*, int, int, int, int)> callback_key;
    CallbackList<void(GLFWwindow*, double, double)> callback_scroll;
    uint32_t getSwapChainImageCount() const {
        return this->vulkanInfo.swapchainImageViews.size();
    }
    // Window 相关函数

    // 获取当前帧的开始时间
    double getCurrentFrameTime() const { return this->windowInfo.currentTime; }
    float getCurrentFrameTimef() const {
        return static_cast<float>(this->windowInfo.currentTime);
    }
    // 获取上一帧的持续时间
    double getDeltaFrameTime() const { return this->windowInfo.deltaTime; }
    float getDeltaFrameTimef() const {
        return static_cast<float>(this->windowInfo.deltaTime);
    }
    void isWindowFullScreen() const { return this->windowInfo.isFullScreen; }
    void isWindowFpsDisplayed() const {
        return this->windowInfo.isFpsDisplayed_debug;
    }
    void setWindowInit(int w, int h, bool isFpsDisplayed);
    void setWindowTitle(const char* newTitle);
    void setWindowTitle(const std::string& newTitle);
    void updateInfo(); /*此函数同时处理FPS显示*/
    bool initWindow(const char* title, bool fullScreen, bool isResizable);
    void setWindowFullSrceen();
    void setWindowWindowed(int offsetX, int offsetY, int width, int height);
    bool checkWindowClose();
    bool terminateWindow();
    inline bool checkWindowClose() {
        return glfwWindowShouldClose(CurContext().windowInfo.pWindow);
    }
};
const uint32_t MAX_FLIGHT_NUM = 3;
extern Context context;
inline Context& CurContext() {
    return context;
}
const double FPS_DISPLAY_DELTA_TIME = 1.0; /*FPS显示间隔时间，以秒为单位*/
const bool PRINT_DEVICE_EXTENSIONS = false;
/*
 * 渲染初始化相关函数
 */
bool initVulkan();
void terminateVulkan();
VkResult recreateSwapchain(); /*重建交换链到当前窗口表面大小*/
void waitAll();
void _setDeviceCreateInfoPNexets(VkPhysicalDeviceProperties2* prooerties);
void _destroyHandles();
void _clearHandles();
std::vector<const char*> _getInstanceExtension();
std::vector<const char*> _getInstanceLayer();
bool _createInstance();
#ifdef BL_DEBUG
VkResult _createDebugMessenger();
#endif  // BL_DEBUG
VkResult _createSurface();
VkResult _getPhysicalDevices(
    std::vector<VkPhysicalDevice>& avaliablePhyDevices);
VkResult _getQueueFamilyIndices(VkPhysicalDevice physicalDevice);
VkResult _selectPhysicalDevice();
std::vector<VkExtensionProperties> _getDeviceExtensions();
std::vector<const char*> _selectDeviceExtensions(
    const std::vector<VkExtensionProperties>& supported_ext);
VkResult _createDevice(VmaAllocatorCreateFlags* out_flag);
VkResult _getSurfaceFormats(std::vector<VkSurfaceFormatKHR>& formats);
VkResult _setSurfaceFormat(
    VkSurfaceFormatKHR surfaceFormat,
    std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);
VkResult _createSwapchain(bool limitFrameRate = true,
                          VkSwapchainCreateFlagsKHR flags = 0);
VkResult _createSwapChain_Internal();
/*
 * VMA 相关函数
 */
VmaAllocatorCreateFlags _getVMAflags(const std::vector<const char*>& ext);
VkResult _initVMA(VmaAllocatorCreateFlags flag);
void _terminateVMA();
}  // namespace BL
#endif  //! BOUNDLESS_INIT_FILE