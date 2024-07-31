#ifndef BOUNDLESS_INIT_FILE
#define BOUNDLESS_INIT_FILE
#include <vector>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <functional>
#include <map>
#include "log.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
namespace BL {
struct WindowContext {
    GLFWwindow* pWindow;
    GLFWmonitor* pMonitor;
    const char* pTitle; /*指向窗口标题的指针，必须保持有效*/
    uint32_t width, height;
    bool isFullScreen = false;
    bool isFpsDisplayed = false;
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
    VkPhysicalDeviceProperties phyDeviceProperties;
    VkPhysicalDeviceMemoryProperties phyDeviceMemoryProperties;
#ifdef BL_DEBUG
    VkDebugUtilsMessengerEXT debugger;
#endif //BL_DEBUG
    VmaAllocator allocator;
};
struct Context {
    WindowContext windowInfo;
    VulkanContext vulkanInfo;
    std::map<int,std::function<void()> > callbacks_createSwapchain;
    std::map<int,std::function<void()> > callbacks_destroySwapchain;
    uint32_t getSwapChainImageCount() const {
        return this->vulkanInfo.swapchainImageViews.size();
    }
};
extern Context context;
void _iterateCallback_CreateSwapchain();
void _iterateCallback_DestroySwapchain();
int addCallback_CreateSwapchain(std::function<void()> p);
int addCallback_DestroySwapchain(std::function<void()> p);
void removeCallback_CreateSwapchain(int id);
void removeCallback_DestroySwapchain(int id);
/*
* 窗口相关函数
*/
const double FPS_DISPLAY_DELTA_TIME = 1.0; /*FPS显示间隔时间，以秒为单位*/
const bool PRINT_DEVICE_EXTENSIONS = false;
void setWindowInit(int w, int h, bool isFpsDisplayed);
void setWindowTitle(
    const char* newTitle); /*newTitle 指针必须在下一次调用前保持可用*/
void calcFps();            /*此函数同时处理FPS显示*/
bool initWindow(const char* title, bool fullScreen, bool isResizable);
void setWindowFullSrceen();
void setWindowWindowed(int offsetX, int offsetY, int width, int height);
bool checkWindowClose();
bool terminateWindow();
inline bool checkWindowClose() {
    return glfwWindowShouldClose(context.windowInfo.pWindow);
}
/*
* 渲染初始化相关函数
*/
bool initVulkan();
void terminateVulkan();
VkResult recreateSwapchain();/*重建交换链到当前窗口表面大小*/
void waitAll();
void _destroyHandles();
void _clearHandles();
std::vector<const char*> _getInstanceExtension();
std::vector<const char*> _getInstanceLayer();
bool _createInstance();
#ifdef BL_DEBUG
VkResult _createDebugMessenger();
#endif //BL_DEBUG
VkResult _createSurface();
VkResult _getPhysicalDevices(std::vector<VkPhysicalDevice>& avaliablePhyDevices);
VkResult _getQueueFamilyIndices(VkPhysicalDevice physicalDevice);
VkResult _selectPhysicalDevice();
std::vector<VkExtensionProperties> _getDeviceExtensions();
std::vector<const char*> _selectDeviceExtensions(const std::vector<VkExtensionProperties>& supported_ext);
VkResult _createDevice(VmaAllocatorCreateFlags* out_flag);
VkResult _getSurfaceFormats(std::vector<VkSurfaceFormatKHR>& formats);
VkResult _setSurfaceFormat(VkSurfaceFormatKHR surfaceFormat,std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);
VkResult _createSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0);
VkResult _createSwapChain_Internal();
/*
 * VMA 相关函数
 */
VmaAllocatorCreateFlags _getVMAflags(const std::vector<const char *>& ext);
VkResult _initVMA(VmaAllocatorCreateFlags flag);
void _terminateVMA();
}  // namespace BL
#endif  //! BOUNDLESS_INIT_FILE