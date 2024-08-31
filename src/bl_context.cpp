#include "bl_context.hpp"

namespace BL {
// Context Class
Context _context;
VkResult Context::getApiVersion(uint32_t* apiVersion) {
    if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))
        return vkEnumerateInstanceVersion(apiVersion);
    else
        *apiVersion = VK_API_VERSION_1_0;
    return VK_SUCCESS;
}
std::vector<const char*> Context::_getInstanceExtension(
    const ContextInstanceInit* pInit) {
    std::vector<const char*> extensions;
    uint32_t extensionCount = 0;
    const char** extensionNames;
    extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (!extensionNames) {
        print_error("InitVulkan", "Get Vulkan extensions failed!");
    }
    for (size_t i = 0; i < extensionCount; i++)
        extensions.push_back(extensionNames[i]);
#ifdef BL_DEBUG
    if (pInit->isDebuging) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#else
    if (pInit->isDebuging) {
        print_warning("_getInstanceExtension",
                      "debuging open but BL_DEBUG not defined!");
    }
#endif  // BL_DEBUG
    for (size_t i = 0; i < pInit->extensionCount; i++) {
        bool flag = true;
        for (size_t j = 0; j < extensions.size(); j++)
            if (std::strcmp(pInit->ppExtensionNames[i], extensions[j]) == 0) {
                flag = false;
                break;
            }
        if (flag)
            extensions.push_back(pInit->ppExtensionNames[i]);
    }
    return extensions;
}
std::vector<const char*> Context::_getInstanceLayer(
    const ContextInstanceInit* pInit) {
    std::vector<const char*> layers;
#ifdef BL_DEBUG
    if (pInit->isDebuging) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
#else
    if (pInit->isDebuging) {
        print_warning("_getInstanceLayer",
                      "debuging open but BL_DEBUG not defined!");
    }
#endif  // BL_DEBUG
    for (size_t i = 0; i < pInit->layerCount; i++) {
        bool flag = true;
        for (size_t j = 0; j < layers.size(); j++)
            if (std::strcmp(pInit->ppLayerNames[i], layers[j]) == 0) {
                flag = false;
                break;
            }
        if (flag)
            layers.push_back(pInit->ppLayerNames[i]);
    }
    return layers;
}
VkResult Context::_createDebugMessenger() {
    static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback =
        [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
           void* pUserData) -> VkBool32 {
        std::cout << pCallbackData->pMessage << '\n';
        return VK_FALSE;
    };
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugUtilsMessengerCallback};
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (vkCreateDebugUtilsMessenger) {
        VkResult result = vkCreateDebugUtilsMessenger(
            instance, &debugUtilsMessengerCreateInfo, nullptr, &debugger);
        if (result)
            print_error("_createDebugMessenger",
                        "Failed to create debug messenger! Error code:",
                        int32_t(result));
        return result;
    }
    print_error("_createDebugMessenger",
                "Failed to get the function pointer of "
                "vkCreateDebugUtilsMessengerEXT!");
    return VK_RESULT_MAX_ENUM;
}
bool Context::createInstance(const ContextInstanceInit* pInit) {
    uint32_t curApiVersion = 0u;
    getApiVersion(&curApiVersion);
    if (curApiVersion < pInit->minApiVersion) {
        print_error("createInstance", "Vulkan api version too low!");
    }
    vulkanApiVersion = std::max(curApiVersion, pInit->minApiVersion);
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = pInit->pAppName,
        .applicationVersion = pInit->appVersion,
        .pEngineName = "bl",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = vulkanApiVersion,
    };

    auto extensions = _getInstanceExtension(pInit);
    auto layers = _getInstanceLayer(pInit);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = uint32_t(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = uint32_t(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()};

    if (VkResult result = vkCreateInstance(&createInfo, nullptr, &instance)) {
        print_error("createInstance", "Vulkan instance create failed",
                    int32_t(result));
        return false;
    }
#ifdef BL_DEBUG
    if (pInit->isDebuging) {
        if (_createDebugMessenger() != VK_SUCCESS)
            print_error("createInstance", "create debug failed!");
    }
#else
    if (pInit->isDebuging) {
        print_warning("createInstance",
                      "debuging open but BL_DEBUG not defined!");
    }
#endif  // BL_DEBUG
    print_log("VulkanInit",
              "Vulkan API Version:", VK_API_VERSION_MAJOR(vulkanApiVersion),
              VK_API_VERSION_MINOR(vulkanApiVersion),
              VK_API_VERSION_PATCH(vulkanApiVersion));
    return true;
}
VkResult Context::_getPhysicalDevices(
    std::vector<VkPhysicalDevice>& avaliablePhyDevices) {
    uint32_t deviceCount;
    if (VkResult result =
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
        print_error("_getPhysicalDevices",
                    "Failed to get the count of physical devices! "
                    "Error code: ",
                    int32_t(result));
        return result;
    }
    if (!deviceCount) {
        print_error("_getPhysicalDevices",
                    "Failed to find any physical device supports "
                    "vulkan!");
        abort();
    }
    avaliablePhyDevices.resize(deviceCount);
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount,
                                                 avaliablePhyDevices.data());
    if (result)
        print_error("_getPhysicalDevices",
                    "Failed to enumerate physical devices! Error code: ",
                    int32_t(result));
    return result;
}
VkResult Context::_getQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                         const ContextDeviceInit* pInit) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);
    if (queueFamilyCount == 0)
        return VK_RESULT_MAX_ENUM;
    std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(
        queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilyPropertieses.data());
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        VkBool32 supportGraphics = queueFamilyPropertieses[i].queueFlags &
                                   VK_QUEUE_GRAPHICS_BIT,
                 supportPresentation = VK_TRUE,
                 supportCompute = queueFamilyPropertieses[i].queueFlags &
                                  VK_QUEUE_COMPUTE_BIT;
        if (pInit->surfaceCount > 0)
            for (uint32_t j = 0; j < pInit->surfaceCount; j++) {
                VkBool32 support;
                if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
                        physicalDevice, i, pInit->surface[j],
                        &support)) {
                    print_error("_getQueueFamilyIndices",
                                "Failed to check physical device "
                                "surface support! Error code: ",
                                int32_t(result));
                    return result;
                }
                supportPresentation = supportPresentation & support;
            }
        if (supportGraphics && supportCompute &&
            (!pInit->surface || supportPresentation)) {
            queueFamilyIndex_graphics = queueFamilyIndex_compute = i;
            if (pInit->surface)
                queueFamilyIndex_presentation = i;
            return VK_SUCCESS;
        }
    }
    return VK_RESULT_MAX_ENUM;
}
VkResult Context::_selectPhysicalDevice(const ContextDeviceInit* pInit) {
    std::vector<VkPhysicalDevice> avaliablePhyDevices;
    if (VkResult result = _getPhysicalDevices(avaliablePhyDevices)) {
        return result;
    }
    bool flag = false;
    if (pInit->isDeviceSuitable)
        for (size_t i = 0; i < avaliablePhyDevices.size(); i++)
            if (pInit->isDeviceSuitable(avaliablePhyDevices[i]))
                phyDevice = avaliablePhyDevices[i], flag = true;
    if (!flag)
        phyDevice = avaliablePhyDevices[0];
    if (VkResult result = _getQueueFamilyIndices(phyDevice, pInit)) {
        return result;
    }
    return VK_SUCCESS;
}
const uint32_t vma_extcheck_count = 4;
static const char* vma_extensionNames[vma_extcheck_count] = {
    VK_KHR_MAINTENANCE_4_EXTENSION_NAME, VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
    VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME};
static const VmaAllocatorCreateFlagBits vma_ext_flags[vma_extcheck_count] = {
    VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT,
    VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,
    VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
    VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT};
static VmaAllocatorCreateFlagBits vma_extensions;
std::vector<const char*> Context::_selectDeviceExtensions(
    const std::vector<VkExtensionProperties>& supported_ext) {
    std::vector<const char*> extensions{"VK_KHR_swapchain"};
    extensions.reserve(extensions.size() + supported_ext.size());
    for (const VkExtensionProperties& it : supported_ext) {
        for (uint32_t i = 0; i < vma_extcheck_count; i++) {
            if (std::strcmp(vma_extensionNames[i], it.extensionName) == 0) {
                extensions.push_back(it.extensionName);
                vma_extensions = VmaAllocatorCreateFlagBits(vma_extensions |
                                                            vma_ext_flags[i]);
            }
        }
    }
    return extensions;
}
std::vector<VkExtensionProperties> Context::_getDeviceExtensions(
    const ContextDeviceInit* pInit) {
    std::vector<VkExtensionProperties> ext;
    uint32_t ext_count = 0;
    VkResult res = vkEnumerateDeviceExtensionProperties(phyDevice, nullptr,
                                                        &ext_count, nullptr);
    if (res != VK_SUCCESS) {
        print_error("getDeviceExtensions", "enumerate device ext failed!");
    }
    ext.resize(ext_count);
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &ext_count,
                                         ext.data());
    if (pInit->debug_print_deviceExtension) {
        print_log("Info",
                  "Supported Device Extensions list: count:", ext_count);
        for (const VkExtensionProperties& it : ext) {
            print_log("Info", '\t', it.extensionName);
        }
    }
    return ext;
}
VkResult Context::_initVMA(const ContextDeviceInit* pInit) {
    VmaAllocatorCreateInfo create_info = {};
    create_info.flags = pInit->vmaFlags | vma_extensions;
    create_info.vulkanApiVersion = vulkanApiVersion;
    create_info.instance = instance;
    create_info.device = device;
    create_info.physicalDevice = phyDevice;
    VkResult res = vmaCreateAllocator(&create_info, &allocator);
    if (res != VK_SUCCESS) {
        print_error("_initVMA",
                    "vma allocator create failed! Error Code:", int32_t(res));
        return res;
    }
    print_log("Info", "VMA create successfull.");
    return VK_SUCCESS;
}
bool Context::createDevice(const ContextDeviceInit* pInit) {
    float queuePriority = 1.0f;
    if (_selectPhysicalDevice(pInit))
        return false;
    // 1.构建队列创建表
    VkDeviceQueueCreateInfo queue_create_infos[3] = {
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority},
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority},
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority}};
    uint32_t queue_create_info_count = 0;
    uint32_t& queue_index_graphics = queueFamilyIndex_graphics;
    uint32_t& queue_index_compute = queueFamilyIndex_compute;
    uint32_t& queue_index_present = queueFamilyIndex_presentation;
    if (queue_index_graphics != VK_QUEUE_FAMILY_IGNORED)
        queue_create_infos[queue_create_info_count++].queueFamilyIndex =
            queue_index_graphics;
    if (queue_index_present != VK_QUEUE_FAMILY_IGNORED &&
        queue_index_present != queue_index_graphics)
        queue_create_infos[queue_create_info_count++].queueFamilyIndex =
            queue_index_present;
    if (queue_index_compute != VK_QUEUE_FAMILY_IGNORED &&
        queue_index_compute != queue_index_graphics &&
        queue_index_compute != queue_index_present)
        queue_create_infos[queue_create_info_count++].queueFamilyIndex =
            queue_index_compute;
    // 2.检查设备特性和扩展
    //   设备属性:
    if (vulkanApiVersion >= VK_API_VERSION_1_1) {
        phyDeviceProperties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        phyDeviceVulkan11Properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
        phyDeviceVulkan12Properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
        phyDeviceVulkan13Properties = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
        if (vulkanApiVersion >= VK_API_VERSION_1_2) {
            phyDeviceProperties.pNext = &phyDeviceVulkan11Properties;
            phyDeviceVulkan11Properties.pNext = &phyDeviceVulkan12Properties;
            if (vulkanApiVersion >= VK_API_VERSION_1_3) {
                phyDeviceVulkan12Properties.pNext =
                    &phyDeviceVulkan13Properties;
            }
        }
        vkGetPhysicalDeviceProperties2(phyDevice, &phyDeviceProperties);
        phyDeviceMemoryProperties = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        vkGetPhysicalDeviceMemoryProperties2(phyDevice,
                                             &phyDeviceMemoryProperties);
    } else {
        vkGetPhysicalDeviceProperties(phyDevice,
                                      &phyDeviceProperties.properties);
        vkGetPhysicalDeviceMemoryProperties(
            phyDevice, &phyDeviceMemoryProperties.memoryProperties);
    }
    //   设备特性:
    if (vulkanApiVersion >= VK_API_VERSION_1_1) {
        phyDeviceFeatures = {.sType =
                                 VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        phyDeviceVulkan11Features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        phyDeviceVulkan12Features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        phyDeviceVulkan13Features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        if (vulkanApiVersion >= VK_API_VERSION_1_2) {
            phyDeviceFeatures.pNext = &phyDeviceVulkan11Features;
            phyDeviceVulkan11Features.pNext = &phyDeviceVulkan12Features;
            if (vulkanApiVersion >= VK_API_VERSION_1_3)
                phyDeviceVulkan12Features.pNext = &phyDeviceVulkan13Features;
        }
        vkGetPhysicalDeviceFeatures2(phyDevice, &phyDeviceFeatures);
    } else
        vkGetPhysicalDeviceFeatures(phyDevice, &phyDeviceFeatures.features);
    //   设备扩展:
    std::vector<VkExtensionProperties> supported_ext =
        _getDeviceExtensions(pInit);
    std::vector<const char*> deviceExtensions =
        _selectDeviceExtensions(supported_ext);
    // 3.创建逻辑设备
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .flags = 0,
        .queueCreateInfoCount = queue_create_info_count,
        .pQueueCreateInfos = queue_create_infos,
        .enabledExtensionCount = uint32_t(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()};
    if (vulkanApiVersion >= VK_API_VERSION_1_1) {
        deviceCreateInfo.pNext = &phyDeviceFeatures;
        // deviceCreateInfo.pEnabledFeatures = nullptr;
    } else {
        // deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.pEnabledFeatures = &phyDeviceFeatures.features;
    }
    if (VkResult result =
            vkCreateDevice(phyDevice, &deviceCreateInfo, nullptr, &device)) {
        print_error("createDevice",
                    "Failed to create a vulkan logical device! "
                    "Error code: ",
                    int32_t(result));
        return result;
    }
    // 4.获取队列
    if (queue_index_graphics != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(device, queue_index_graphics, 0, &queue_graphics);
    if (queue_index_present != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(device, queue_index_present, 0, &queue_presentation);
    if (queue_index_compute != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(device, queue_index_compute, 0, &queue_compute);
    print_log("Init", "Renderer:", phyDeviceProperties.properties.deviceName);
    return VK_SUCCESS;
}
void Context::destroyDevice() {
    if (allocator) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }
    if (device) {
        vkDeviceWaitIdle(device);
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
    phyDevice = VK_NULL_HANDLE;
}
void Context::destroyInstance() {
#ifdef BL_DEBUG
    if (debugger) {
        PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance,
                                      "vkDestroyDebugUtilsMessengerEXT"));
        if (DestroyDebugUtilsMessenger)
            DestroyDebugUtilsMessenger(instance, debugger, nullptr);
        debugger = VK_NULL_HANDLE;
    }
#endif  // BL_DEBUG
    if (instance) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}
// WindowContext Class
static void _glfw_error_callback(int error, const char* description) {
    print_error("GLFW", error, ':', description);
}
static void _glfw_size_callback(GLFWwindow* window, int width, int height) {
    WindowContext* cxt = (WindowContext*)glfwGetWindowUserPointer(window);
    print_log("WindowSize", "Window Name:", cxt->title,
              "\tNew window size(w/h):", width, height);
}
static void _glfw_cursorPos_callback(GLFWwindow* pWindow, double x, double y) {
    WindowContext* cxt = (WindowContext*)glfwGetWindowUserPointer(pWindow);
    cxt->callback_cursorPos.iterate(pWindow, x, y);
}
static void _glfw_keyboard_callback(GLFWwindow* pWindow,
                                    int key,
                                    int scancode,
                                    int action,
                                    int mods) {
    WindowContext* cxt = (WindowContext*)glfwGetWindowUserPointer(pWindow);
    cxt->callback_key.iterate(pWindow, key, scancode, action, mods);
}
static void _glfw_scroll_callback(GLFWwindow* pWindow,
                                  double xoffset,
                                  double yoffset) {
    WindowContext* cxt = (WindowContext*)glfwGetWindowUserPointer(pWindow);
    cxt->callback_scroll.iterate(pWindow, xoffset, yoffset);
}
// WindowContext::WindowContext(WindowContext&& other) {
//     pWindow = other.pWindow;
//     pMonitor = other.pMonitor;
//     title = std::move(other.title);
//     lastWidth = other.lastWidth;
//     lastHeight = other.lastHeight;
//     lastPosX = other.lastPosX;
//     lastPosY = other.lastPosY;
//     maxWidth = other.maxWidth;
//     minWidth = other.minWidth;
//     maxHeight = other.maxHeight;
//     minHeight = other.minHeight;
//     currentTime = other.currentTime;
//     deltaTime = other.deltaTime;
//     isFullScreen = other.isFullScreen;
//     isFpsShowed = other.isFpsShowed;
//     deltaFrame = other.deltaFrame;
//     stringBuilder = std::move(other.stringBuilder);
//     lastTime = other.lastTime;

//     callback_createSwapchain = std::move(other.callback_createSwapchain);
//     callback_destroySwapchain = std::move(other.callback_destroySwapchain);
//     callback_cursorPos = std::move(other.callback_cursorPos);
//     callback_key = std::move(other.callback_key);
//     callback_scroll = std::move(other.callback_scroll);

//     surface = other.surface;
//     swapchain = other.swapchain;
//     swapchainImages = std::move(other.swapchainImages);
//     swapchainImageViews = std::move(other.swapchainImageViews);
//     swapchainCreateInfo = other.swapchainCreateInfo;

//     other.pWindow = nullptr;
//     other.pMonitor = nullptr;
//     other.surface = VK_NULL_HANDLE;
//     other.swapchain = VK_NULL_HANDLE;
//     other.swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
//     glfwSetWindowUserPointer(pWindow, this);
// }
bool WindowContext::createWindow(const WindowContextInit* pInit) {
    pMonitor = glfwGetPrimaryMonitor();
    if (pMonitor == nullptr) {
        print_error("WindowContext", "No Monitor!");
        return false;
    }
    title = pInit->initTitle;
    lastWidth = pInit->initWidth;
    lastHeight = pInit->initHeight;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, pInit->isResizable);
    if (pInit->isFullScreen) {
        isFullScreen = true;
        const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
        pWindow = glfwCreateWindow(pMode->width, pMode->height, title.c_str(),
                                   pMonitor, nullptr);
    } else {
        pWindow = glfwCreateWindow(pInit->initWidth, pInit->initHeight,
                                   title.c_str(), nullptr, nullptr);
    }
    if (pWindow == nullptr) {
        print_error("WindowContext", "Window create failed!");
        return false;
    }
    maxWidth = pInit->maxWidth;
    minWidth = pInit->minWidth;
    maxHeight = pInit->maxHeight;
    minHeight = pInit->minHeight;
    glfwSetWindowSizeLimits(pWindow, minWidth, minHeight, maxWidth, maxHeight);
    currentTime = glfwGetTime();
    setFpsShowed(pInit->isFpsShowed);
    setCursorVisable(pInit->isCursorVisable);
    glfwSetWindowUserPointer(pWindow, this);
    glfwSetWindowSizeCallback(pWindow, _glfw_size_callback);
    glfwSetCursorPosCallback(pWindow, _glfw_cursorPos_callback);
    glfwSetKeyCallback(pWindow, _glfw_keyboard_callback);
    glfwSetScrollCallback(pWindow, _glfw_scroll_callback);
    if (_createSurface())
        return false;
    return true;
}
void WindowContext::update() {
    double cur_time = glfwGetTime();
    deltaTime = cur_time - currentTime;
    currentTime = cur_time;
    if (isFpsShowed) {
        deltaFrame++;
        double delta_time = currentTime - lastTime;
        if (delta_time >= 1.0) {
            if (!isFullScreen) {
                stringBuilder.precision(1);
                stringBuilder << title << "  " << std::fixed
                              << deltaFrame / delta_time << " FPS";
                glfwSetWindowTitle(pWindow, stringBuilder.str().c_str());
                stringBuilder.str("");
            } else {
                glfwSetWindowTitle(pWindow, title.c_str());
            }
            lastTime = currentTime;
            deltaFrame = 0;
        }
    }
}
void WindowContext::destroy() {
    auto& info = CurContext();
    if (swapchain) {
        callback_destroySwapchain.iterate();
        for (auto& i : swapchainImageViews)
            if (i)
                vkDestroyImageView(info.device, i, nullptr);
        vkDestroySwapchainKHR(info.device, swapchain, nullptr);
        swapchainImageViews.clear();
        swapchain = VK_NULL_HANDLE;
    }
    if (surface) {
        vkDestroySurfaceKHR(info.instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if (pWindow)
        glfwDestroyWindow(pWindow);
    pWindow = nullptr;
    pMonitor = nullptr;
}

void WindowContext::setTitle(const char* newTitle) {
    glfwSetWindowTitle(pWindow, newTitle);
}
void WindowContext::setTitle(const std::string& newTitle) {
    glfwSetWindowTitle(pWindow, newTitle.c_str());
}
void WindowContext::setFullSrceen(bool flag) {
    if (isFullScreen == flag) {
        return;
    }
    isFullScreen = flag;
    if (flag) {
        glfwGetWindowPos(pWindow, (int*)&lastHeight, (int*)&lastHeight);
        glfwGetWindowSize(pWindow, (int*)&lastWidth, (int*)&lastWidth);

        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);
        glfwSetWindowMonitor(pWindow, pMonitor, 0, 0, mode->width, mode->height,
                             0);
    } else {
        glfwSetWindowMonitor(pWindow, nullptr, lastPosX, lastPosY, lastWidth,
                             lastHeight, GLFW_DONT_CARE);
    }
    recreateSwapchain();
}
void WindowContext::setFpsShowed(bool flag) {
    isFpsShowed = flag;
    lastTime = flag ? currentTime : 0.0;
    deltaFrame = 0;
}
void WindowContext::setCursorVisable(bool flag) {
    if (flag) {
        glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}
void WindowContext::setWindowSize(int offsetX,
                                  int offsetY,
                                  int width,
                                  int height) {
    if (!isFullScreen) {
        glfwGetWindowPos(pWindow, (int*)&lastHeight, (int*)&lastHeight);
        glfwGetWindowSize(pWindow, (int*)&lastWidth, (int*)&lastWidth);
        glfwSetWindowMonitor(pWindow, nullptr, offsetX, offsetY, width, height,
                             GLFW_DONT_CARE);
        recreateSwapchain();
    }
}
void WindowContext::setWindowSizeLimits(uint32_t maxWidth,
                                        uint32_t minWidth,
                                        uint32_t maxHeight,
                                        uint32_t minHeight) {
    this->maxWidth = maxWidth;
    this->minWidth = minWidth;
    this->maxHeight = maxHeight;
    this->minHeight = minHeight;
    glfwSetWindowSizeLimits(pWindow, minWidth, minHeight, maxWidth, maxHeight);
}
void WindowContext::setWindowSizeAspect() {
    uint32_t w, h;
    glfwGetWindowSize(pWindow, (int*)&w, (int*)&h);
    glfwSetWindowAspectRatio(pWindow, w, h);
}
bool WindowContext::initialize() {
    glfwSetErrorCallback(_glfw_error_callback);
    if (!glfwInit()) {
        print_error("WindowContext", "GLFW init failed!");
        return false;
    }
    return true;
}
void WindowContext::terminate() {
    glfwTerminate();
}
VkResult WindowContext::_createSurface() {
    if (VkResult result = glfwCreateWindowSurface(CurContext().instance,
                                                  pWindow, nullptr, &surface)) {
        print_error(
            "WindowContext",
            "Window surface create failed! Error code: ", int32_t(result));
        return result;
    }
    return VK_SUCCESS;
}
VkResult WindowContext::_getSurfaceFormats(
    std::vector<VkSurfaceFormatKHR>& formats) {
    auto& info = CurContext();
    uint32_t format_count;
    if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            info.phyDevice, surface, &format_count, nullptr)) {
        print_error("getSurfaceFormats",
                    "Failed to get the count of surface "
                    "formats! Error code:",
                    int32_t(result));
        return result;
    }
    if (!format_count) {
        print_error("getSurfaceFormats",
                    "Failed to find any supported surface "
                    "format!");
        abort();
    }
    formats.resize(format_count);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        info.phyDevice, surface, &format_count, formats.data());
    if (result)
        print_error("getSurfaceFormats",
                    "Failed to get surface formats! Error "
                    "code:",
                    int32_t(result));
    return result;
}
VkResult WindowContext::_setSurfaceFormat(
    VkSurfaceFormatKHR surfaceFormat,
    std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats) {
    auto& createInfo = swapchainCreateInfo;

    bool formatIsAvailable = false;
    if (!surfaceFormat.format) {
        // 如果格式未指定，只匹配色彩空间，图像格式有啥就用啥
        for (auto& i : availableSurfaceFormats)
            if (i.colorSpace == surfaceFormat.colorSpace) {
                createInfo.imageFormat = i.format;
                createInfo.imageColorSpace = i.colorSpace;
                formatIsAvailable = true;
                break;
            }
    } else
        // 否则匹配格式和色彩空间
        for (auto& i : availableSurfaceFormats)
            if (i.format == surfaceFormat.format &&
                i.colorSpace == surfaceFormat.colorSpace) {
                createInfo.imageFormat = i.format;
                createInfo.imageColorSpace = i.colorSpace;
                formatIsAvailable = true;
                break;
            }
    if (!formatIsAvailable)
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    // 如果交换链已存在，重建交换链
    if (swapchain)
        return recreateSwapchain();
    return VK_SUCCESS;
}
VkResult WindowContext::_createSwapChain_Internal() {
    auto& context = CurContext();
    auto& createInfo = swapchainCreateInfo;
    if (VkResult result = vkCreateSwapchainKHR(context.device, &createInfo,
                                               nullptr, &swapchain)) {
        print_error("createSwapchain",
                    "Failed to create a swapchain! Error "
                    "code:",
                    int32_t(result));
        return result;
    }
    uint32_t swapchainImageCount;
    if (VkResult result = vkGetSwapchainImagesKHR(
            context.device, swapchain, &swapchainImageCount, nullptr)) {
        print_error("createSwapchain",
                    "Failed to get the count of swapchain images! Error code:",
                    int32_t(result));
        return result;
    }
    swapchainImages.resize(swapchainImageCount);
    if (VkResult result = vkGetSwapchainImagesKHR(context.device, swapchain,
                                                  &swapchainImageCount,
                                                  swapchainImages.data())) {
        print_error(
            "createSwapchain",
            "Failed to get swapchain images! Error code:", int32_t(result));
        return result;
    }

    swapchainImageViews.resize(swapchainImageCount);
    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = createInfo.imageFormat,
        //.components = {},//四个成员皆为VK_COMPONENT_SWIZZLE_IDENTITY
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    for (size_t i = 0; i < swapchainImageCount; i++) {
        imageViewCreateInfo.image = swapchainImages[i];
        if (VkResult result =
                vkCreateImageView(context.device, &imageViewCreateInfo, nullptr,
                                  &swapchainImageViews[i])) {
            print_error("createSwapchain",
                        "Failed to create a swapchain image view! Error code:",
                        int32_t(result));
            return result;
        }
    }
    return VK_SUCCESS;
}
VkResult WindowContext::createSwapchain(
    const WindowContextSwapchainInit* pInit) {
    auto& context = CurContext();
    VkSurfaceCapabilitiesKHR surface_capabilities;
    if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            context.phyDevice, surface, &surface_capabilities)) {
        print_error(
            "createSwapchain",
            "Failed to get physical device surface capabilities! Error code:",
            int32_t(result));
        return result;
    }
    auto& createInfo = swapchainCreateInfo;
    createInfo.minImageCount = surface_capabilities.minImageCount +
                               (surface_capabilities.maxImageCount >
                                surface_capabilities.minImageCount);
    uint32_t width, height;
    glfwGetWindowSize(pWindow, (int*)&width, (int*)&height);
    createInfo.imageExtent =
        surface_capabilities.currentExtent.width == (~0u)
            ? VkExtent2D{std::clamp(width,
                                    surface_capabilities.minImageExtent.width,
                                    surface_capabilities.maxImageExtent.width),
                         std::clamp(height,
                                    surface_capabilities.minImageExtent.height,
                                    surface_capabilities.maxImageExtent.height)}
            : surface_capabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.preTransform = surface_capabilities.currentTransform;
    if (surface_capabilities.supportedCompositeAlpha &
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    else
        for (size_t i = 0; i < 4; i++)
            if (surface_capabilities.supportedCompositeAlpha & 1 << i) {
                createInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(
                    surface_capabilities.supportedCompositeAlpha & 1 << i);
                break;
            }
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (surface_capabilities.supportedUsageFlags &
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (surface_capabilities.supportedUsageFlags &
        VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    else
        print_warning("createSwapchain",
                      "VK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported!");
    // 指定图像格式
    static std::vector<VkSurfaceFormatKHR> availableFormats;
    if (availableFormats.empty())
        if (VkResult result = _getSurfaceFormats(availableFormats))
            return result;
    if (!createInfo.imageFormat)
        if (_setSurfaceFormat(
                {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                availableFormats) &&
            _setSurfaceFormat(
                {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                availableFormats)) {
            // 如果找不到上述图像格式和色彩空间的组合，那只能有什么用什么，采用availableSurfaceFormats中的第一组
            createInfo.imageFormat = availableFormats[0].format;
            createInfo.imageColorSpace = availableFormats[0].colorSpace;
            print_warning(
                "createSwapchain",
                "Failed to select a four-component UNORM surface format!\n");
        }
    // 指定呈现模式
    uint32_t surfacePresentModeCount;
    if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            context.phyDevice, surface, &surfacePresentModeCount, nullptr)) {
        print_error(
            "createSwapchain",
            "Failed to get the count of surface present modes! Error code:",
            int32_t(result));
        return result;
    }
    if (!surfacePresentModeCount) {
        print_error("createSwapchain",
                    "Failed to find any surface present mode!");
        abort();
    }
    std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
    if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            context.phyDevice, surface, &surfacePresentModeCount,
            surfacePresentModes.data())) {
        print_error("createSwapchain",
                    "Failed to get surface present "
                    "modes! Error code:",
                    int32_t(result));
        return result;
    }
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!pInit->isFrameRateLimited)
        for (size_t i = 0; i < surfacePresentModeCount; i++)
            if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
    const char* mode_str;
    switch (createInfo.presentMode) {
        case VK_PRESENT_MODE_FIFO_KHR:
            mode_str = "VK_PRESENT_MODE_FIFO_KHR";
            break;
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            mode_str = "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
            break;
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            mode_str = "VK_PRESENT_MODE_IMMEDIATE_KHR";
            break;
        case VK_PRESENT_MODE_MAILBOX_KHR:
            mode_str = "VK_PRESENT_MODE_MAILBOX_KHR";
            break;
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            mode_str = "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
            break;
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
            mode_str = "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
            break;
        default:
            mode_str = "Unknown!";
            break;
    }
    print_log("Present Mode", mode_str);
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.flags = pInit->flags;
    createInfo.surface = surface;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    createInfo.pNext = nullptr;
    if (VkResult result = _createSwapChain_Internal())
        return result;
    callback_createSwapchain.iterate();
    return VK_SUCCESS;
}
VkResult WindowContext::recreateSwapchain() {
    auto& info = CurContext();
    auto& createInfo = swapchainCreateInfo;
    VkSurfaceCapabilitiesKHR surface_capabilities = {};
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        info.phyDevice, surface, &surface_capabilities);
    if (result != VK_SUCCESS) {
        print_error(
            "recreateSwapchain",
            "Failed to get physical device surface capabilities! Error code:",
            int32_t(result));
        return result;
    }
    if (surface_capabilities.currentExtent.width == 0 ||
        surface_capabilities.currentExtent.height == 0)
        return VK_SUBOPTIMAL_KHR;
    createInfo.imageExtent = surface_capabilities.currentExtent;
    createInfo.oldSwapchain = swapchain;
    result = vkQueueWaitIdle(info.queue_graphics);
    // 仅在等待图形队列成功，且图形与呈现所用队列不同时等待呈现队列
    if (!result && info.queue_graphics != info.queue_presentation)
        result = vkQueueWaitIdle(info.queue_presentation);
    if (result) {
        print_error("recreateSwapchain",
                    "Failed to wait for the queue to be idle! Error code:",
                    int32_t(result));
        return result;
    }
    callback_destroySwapchain.iterate();
    for (auto& i : swapchainImageViews)
        if (i)
            vkDestroyImageView(info.device, i, nullptr);
    swapchainImageViews.resize(0);
    result = _createSwapChain_Internal();
    if (result != VK_SUCCESS) {
        print_error("recreateSwapchain",
                    "Create swapchain failed! Code:", int32_t(result));
        return result;
    }
    callback_createSwapchain.iterate();
    print_log("Info", "Swapchain recreated!");
    return VK_SUCCESS;
}
}  // namespace BL