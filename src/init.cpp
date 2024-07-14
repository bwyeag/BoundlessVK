#include "BL/init.hpp"
namespace BL {
Context context;
/*
 * 窗口相关函数
 */
void setWindowInit(int w, int h, bool isFpsDisplayed) {
    context.windowInfo.width = w;
    context.windowInfo.height = h;
    context.windowInfo.isFpsDisplayed = isFpsDisplayed;
}
void setWindowTitle(const char* newTitle) {
    context.windowInfo.pTitle = newTitle;
    glfwSetWindowTitle(context.windowInfo.pWindow, newTitle);
}
void calcFps() {
    static double last_time = glfwGetTime();
    static double delta_time = 0.0;
    static uint32_t delta_frame = -1;
    static std::stringstream info;
    context.windowInfo.lastTime = context.windowInfo.currentTime;
    context.windowInfo.currentTime = glfwGetTime();
    context.windowInfo.deltaTime =
        context.windowInfo.currentTime - context.windowInfo.lastTime;
    if (context.windowInfo.isFpsDisplayed) {
        delta_frame++;
        if ((delta_time = context.windowInfo.currentTime - last_time) >=
            FPS_DISPLAY_DELTA_TIME) {
            if (!context.windowInfo.isFullScreen) {
                info.precision(1);
                info << context.windowInfo.pTitle << "  " << std::fixed
                     << delta_frame / delta_time << " FPS";
                glfwSetWindowTitle(context.windowInfo.pWindow,
                                   info.str().c_str());
                info.str("");
            } else {
                glfwSetWindowTitle(context.windowInfo.pWindow,
                                   context.windowInfo.pTitle);
            }
            last_time = context.windowInfo.currentTime;
            delta_frame = 0;
        }
    }
}
bool initWindow(const char* title, bool fullScreen, bool isResizable) {
    if (!glfwInit()) {
        print_error("InitWindow", "GLFW init failed!");
        return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    context.windowInfo.pMonitor = glfwGetPrimaryMonitor();
    context.windowInfo.pTitle = title;
    if (fullScreen) {
        context.windowInfo.isFullScreen = true;
        const GLFWvidmode* pMode =
            glfwGetVideoMode(context.windowInfo.pMonitor);
        context.windowInfo.width = pMode->width;
        context.windowInfo.height = pMode->height;
        context.windowInfo.pWindow =
            glfwCreateWindow(pMode->width, pMode->height, title,
                             context.windowInfo.pMonitor, nullptr);
    } else {
        context.windowInfo.pWindow = glfwCreateWindow(context.windowInfo.width,
                                                      context.windowInfo.height,
                                                      title, nullptr, nullptr);
    }
    if (!context.windowInfo.pWindow) {
        print_error("InitWindow", "GLFW window create failed!");
        glfwTerminate();
        return false;
    }
    return true;
}
void setWindowFullSrceen() {
    const GLFWvidmode* pMode = glfwGetVideoMode(context.windowInfo.pMonitor);
    glfwSetWindowMonitor(context.windowInfo.pWindow,
                         context.windowInfo.pMonitor, 0, 0, pMode->width,
                         pMode->height, pMode->refreshRate);
}
void setWindowWindowed(int offsetX, int offsetY, int width, int height) {
    const GLFWvidmode* pMode = glfwGetVideoMode(context.windowInfo.pMonitor);
    glfwSetWindowMonitor(context.windowInfo.pWindow, nullptr, offsetX, offsetY,
                         width, height, pMode->refreshRate);
}
bool terminateWindow() {
    glfwDestroyWindow(context.windowInfo.pWindow);
    glfwTerminate();
    return true;
}
/*
 * 渲染初始化相关函数
 */
bool initVulkan() {
    if (!_createInstance()) {
        return false;
    }
#ifdef BL_DEBUG
    if (_createDebugMessenger()) {
        return false;
    }
#endif  // BL_DEBUG
    if (_createSurface()) {
        return false;
    }
    if (_pickPhysicalDevice()) {
        return false;
    }
    if (_createDevice()) {
        return false;
    }
    if (_createSwapchain()) {
        return false;
    }
    return true;
}
std::vector<const char*> _getInstanceExtension() {
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
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif  // BL_DEBUG
    return extensions;
}
std::vector<const char*> _getInstanceLayer() {
    std::vector<const char*> layers;
#ifdef BL_DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif  // BL_DEBUG
    return layers;
}
bool _createInstance() {
    uint32_t apiVersion = VK_API_VERSION_1_3;
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = apiVersion;
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.pApplicationName = "Triangle";
    appInfo.pEngineName = "None";

    auto extensions = _getInstanceExtension();
    auto layers = _getInstanceLayer();

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = uint32_t(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = uint32_t(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &context.vulkanInfo.instance) !=
        VK_SUCCESS) {
        print_error("InitVulkan", "Vulkan instance create failed");
        return false;
    }
    print_log("Init", "Vulkan API Version:", VK_API_VERSION_MAJOR(apiVersion),
              VK_API_VERSION_MINOR(apiVersion),
              VK_API_VERSION_PATCH(apiVersion));
    return true;
}
#ifdef BL_DEBUG
VkResult _createDebugMessenger() {
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
            vkGetInstanceProcAddr(context.vulkanInfo.instance,
                                  "vkCreateDebugUtilsMessengerEXT"));
    if (vkCreateDebugUtilsMessenger) {
        VkResult result = vkCreateDebugUtilsMessenger(
            context.vulkanInfo.instance, &debugUtilsMessengerCreateInfo,
            nullptr, &context.vulkanInfo.debugger);
        if (result)
            print_error("InitVulkan",
                        "Failed to create debug messenger! Error code:",
                        int32_t(result));
        return result;
    }
    print_error("InitVulkan",
                "Failed to get the function pointer of "
                "vkCreateDebugUtilsMessengerEXT!");
    return VK_RESULT_MAX_ENUM;
}
#endif  // BL_DEBUG
VkResult _createSurface() {
    if (VkResult result = glfwCreateWindowSurface(
            context.vulkanInfo.instance, context.windowInfo.pWindow, nullptr,
            &context.vulkanInfo.surface)) {
        print_error("InitVulkan", "Window surface create failed! Error code: ",
                    int32_t(result));
        return result;
    }
    return VK_SUCCESS;
}
VkResult _getPhysicalDevices(
    std::vector<VkPhysicalDevice>& avaliablePhyDevices) {
    uint32_t deviceCount;
    if (VkResult result = vkEnumeratePhysicalDevices(
            context.vulkanInfo.instance, &deviceCount, nullptr)) {
        print_error("InitVulkan",
                    "Failed to get the count of physical devices! "
                    "Error code: ",
                    int32_t(result));
        return result;
    }
    if (!deviceCount) {
        print_error("InitVulkan",
                    "Failed to find any physical device supports "
                    "vulkan!");
        abort();
    }
    avaliablePhyDevices.resize(deviceCount);
    VkResult result = vkEnumeratePhysicalDevices(
        context.vulkanInfo.instance, &deviceCount, avaliablePhyDevices.data());
    if (result)
        print_error("InitVulkan",
                    "Failed to enumerate physical devices! Error code: ",
                    int32_t(result));
    return result;
}
VkResult _getQueueFamilyIndices(VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);
    if (!queueFamilyCount)
        return VK_RESULT_MAX_ENUM;
    std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(
        queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilyPropertieses.data());
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        VkBool32 supportGraphics = queueFamilyPropertieses[i].queueFlags &
                                   VK_QUEUE_GRAPHICS_BIT,
                 supportPresentation = false,
                 supportCompute = queueFamilyPropertieses[i].queueFlags &
                                  VK_QUEUE_COMPUTE_BIT;
        if (context.vulkanInfo.surface)
            if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
                    physicalDevice, i, context.vulkanInfo.surface,
                    &supportPresentation)) {
                print_error("InitVulkan",
                            "Failed to check physical device "
                            "surface support! Error code: ",
                            int32_t(result));
                return result;
            }
        if (supportGraphics && supportCompute &&
            (!context.vulkanInfo.surface || supportPresentation)) {
            context.vulkanInfo.queueFamilyIndex_graphics =
                context.vulkanInfo.queueFamilyIndex_compute = i;
            if (context.vulkanInfo.surface)
                context.vulkanInfo.queueFamilyIndex_presentation = i;
            return VK_SUCCESS;
        }
    }
    return VK_RESULT_MAX_ENUM;
}
VkResult _pickPhysicalDevice() {
    std::vector<VkPhysicalDevice> avaliablePhyDevices;
    if (VkResult result = _getPhysicalDevices(avaliablePhyDevices)) {
        return result;
    }
    context.vulkanInfo.phyDevice = avaliablePhyDevices[0];
    if (VkResult result =
            _getQueueFamilyIndices(context.vulkanInfo.phyDevice)) {
        return result;
    }
    return VK_SUCCESS;
}
std::vector<const char*> _getDeviceExtension() {
    std::vector<const char*> extensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    return extensions;
}
VkResult _createDevice() {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[3] = {
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority},
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority},
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority}};
    uint32_t queueCreateInfoCount = 0;
    uint32_t& qInd_graphics = context.vulkanInfo.queueFamilyIndex_graphics;
    uint32_t& qInd_compute = context.vulkanInfo.queueFamilyIndex_compute;
    uint32_t& qInd_present = context.vulkanInfo.queueFamilyIndex_presentation;
    if (qInd_graphics != VK_QUEUE_FAMILY_IGNORED)
        queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex =
            qInd_graphics;
    if (qInd_present != VK_QUEUE_FAMILY_IGNORED &&
        qInd_present != qInd_graphics)
        queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex =
            qInd_present;
    if (qInd_compute != VK_QUEUE_FAMILY_IGNORED &&
        qInd_compute != qInd_graphics && qInd_compute != qInd_present)
        queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex =
            qInd_compute;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(context.vulkanInfo.phyDevice,
                                &physicalDeviceFeatures);
    auto deviceExtensions = _getDeviceExtension();
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .flags = 0,
        .queueCreateInfoCount = queueCreateInfoCount,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledExtensionCount = uint32_t(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures};
    if (VkResult result =
            vkCreateDevice(context.vulkanInfo.phyDevice, &deviceCreateInfo,
                           nullptr, &context.vulkanInfo.device)) {
        print_error("InitVulkan",
                    "Failed to create a vulkan logical device! "
                    "Error code: ",
                    int32_t(result));
        return result;
    }
    if (qInd_graphics != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(context.vulkanInfo.device, qInd_graphics, 0,
                         &context.vulkanInfo.queue_graphics);
    if (qInd_present != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(context.vulkanInfo.device, qInd_present, 0,
                         &context.vulkanInfo.queue_presentation);
    if (qInd_compute != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(context.vulkanInfo.device, qInd_compute, 0,
                         &context.vulkanInfo.queue_compute);
    vkGetPhysicalDeviceProperties(context.vulkanInfo.phyDevice,
                                  &context.vulkanInfo.phyDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(
        context.vulkanInfo.phyDevice,
        &context.vulkanInfo.phyDeviceMemoryProperties);
    print_log("Init",
              "Renderer:", context.vulkanInfo.phyDeviceProperties.deviceName);
    return VK_SUCCESS;
}
VkResult _getSurfaceFormats(std::vector<VkSurfaceFormatKHR>& formats) {
    uint32_t surfaceFormatCount;
    if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            context.vulkanInfo.phyDevice, context.vulkanInfo.surface,
            &surfaceFormatCount, nullptr)) {
        print_error("InitSwapChain",
                    "Failed to get the count of surface "
                    "formats! Error code:",
                    int32_t(result));
        return result;
    }
    if (!surfaceFormatCount) {
        print_error("InitSwapChain",
                    "Failed to find any supported surface "
                    "format!");
        abort();
    }
    formats.resize(surfaceFormatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        context.vulkanInfo.phyDevice, context.vulkanInfo.surface,
        &surfaceFormatCount, formats.data());
    if (result)
        print_error("InitSwapChain",
                    "Failed to get surface formats! Error "
                    "code:",
                    int32_t(result));
    return result;
}
VkResult _createSwapchain(bool limitFrameRate,
                          VkSwapchainCreateFlagsKHR flags) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            context.vulkanInfo.phyDevice, context.vulkanInfo.surface,
            &surfaceCapabilities)) {
        print_error(
            "createSwapchain",
            "Failed to get physical device surface capabilities!Error code:",
            int32_t(result));
        return result;
    }
    auto& createInfo = context.vulkanInfo.swapchainCreateInfo;
    createInfo.minImageCount =
        surfaceCapabilities.minImageCount +
        (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);
    createInfo.imageExtent =
        surfaceCapabilities.currentExtent.width == -1
            ? VkExtent2D{std::clamp(context.windowInfo.width,
                                    surfaceCapabilities.minImageExtent.width,
                                    surfaceCapabilities.maxImageExtent.width),
                         std::clamp(context.windowInfo.height,
                                    surfaceCapabilities.minImageExtent.height,
                                    surfaceCapabilities.maxImageExtent.height)}
            : surfaceCapabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    if (surfaceCapabilities.supportedCompositeAlpha &
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    else
        for (size_t i = 0; i < 4; i++)
            if (surfaceCapabilities.supportedCompositeAlpha & 1 << i) {
                createInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(
                    surfaceCapabilities.supportedCompositeAlpha & 1 << i);
                break;
            }
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (surfaceCapabilities.supportedUsageFlags &
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (surfaceCapabilities.supportedUsageFlags &
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
        if (_setSurfaceFormat({VK_FORMAT_R8G8B8A8_UNORM,
                              VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},availableFormats) &&
            _setSurfaceFormat({VK_FORMAT_B8G8R8A8_UNORM,
                              VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},availableFormats)) {
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
            context.vulkanInfo.phyDevice, context.vulkanInfo.surface,
            &surfacePresentModeCount, nullptr)) {
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
            context.vulkanInfo.phyDevice, context.vulkanInfo.surface,
            &surfacePresentModeCount, surfacePresentModes.data())) {
        print_error("createSwapchain",
                    "Failed to get surface present "
                    "modes! Error code:",
                    int32_t(result));
        return result;
    }
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!limitFrameRate)
        for (size_t i = 0; i < surfacePresentModeCount; i++)
            if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.flags = flags;
    createInfo.surface = context.vulkanInfo.surface;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.clipped = VK_TRUE;
    if (VkResult result = _createSwapChain_Internal())
        return result;
    for (auto& i : context.callbacks_createSwapchain)
        i();
    return VK_SUCCESS;
}
VkResult _createSwapChain_Internal() {
    auto& createInfo = context.vulkanInfo.swapchainCreateInfo;
    if (VkResult result =
            vkCreateSwapchainKHR(context.vulkanInfo.device, &createInfo,
                                 nullptr, &context.vulkanInfo.swapchain)) {
        print_error("createSwapchain",
                    "Failed to create a swapchain! Error "
                    "code:",
                    int32_t(result));
        return result;
    }
    uint32_t swapchainImageCount;
    if (VkResult result = vkGetSwapchainImagesKHR(
            context.vulkanInfo.device, context.vulkanInfo.swapchain,
            &swapchainImageCount, nullptr)) {
        print_error("createSwapchain",
                    "Failed to get the count of swapchain images! Error code:",
                    int32_t(result));
        return result;
    }
    context.vulkanInfo.swapchainImages.resize(swapchainImageCount);
    if (VkResult result = vkGetSwapchainImagesKHR(
            context.vulkanInfo.device, context.vulkanInfo.swapchain,
            &swapchainImageCount, context.vulkanInfo.swapchainImages.data())) {
        print_error(
            "createSwapchain",
            "Failed to get swapchain images! Error code:", int32_t(result));
        return result;
    }

    context.vulkanInfo.swapchainImageViews.resize(swapchainImageCount);
    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = createInfo.imageFormat,
        //.components = {},//四个成员皆为VK_COMPONENT_SWIZZLE_IDENTITY
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    for (size_t i = 0; i < swapchainImageCount; i++) {
        imageViewCreateInfo.image = context.vulkanInfo.swapchainImages[i];
        if (VkResult result =
                vkCreateImageView(context.vulkanInfo.device, &imageViewCreateInfo, nullptr,
                                  &context.vulkanInfo.swapchainImageViews[i])) {
            print_error("createSwapchain",
                        "Failed to create a swapchain image view! Error code:",
                        int32_t(result));
            return result;
        }
    }
    return VK_SUCCESS;
}
VkResult _setSurfaceFormat(VkSurfaceFormatKHR surfaceFormat,std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats) {
    auto& createInfo = context.vulkanInfo.swapchainCreateInfo;

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
    if (context.vulkanInfo.swapchain)
        return _recreateSwapchain();
    return VK_SUCCESS;
}
VkResult _recreateSwapchain() {
    auto& createInfo = context.vulkanInfo.swapchainCreateInfo;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            context.vulkanInfo.phyDevice, context.vulkanInfo.surface, &surfaceCapabilities)) {
        print_error(
            "recreateSwapchain",
            "Failed to get physical device surface capabilities! Error code:",
            int32_t(result));
        return result;
    }
    if (surfaceCapabilities.currentExtent.width == 0 ||
        surfaceCapabilities.currentExtent.height == 0)
        return VK_SUBOPTIMAL_KHR;
    createInfo.imageExtent = surfaceCapabilities.currentExtent;
    createInfo.oldSwapchain = context.vulkanInfo.swapchain;
    VkResult result = vkQueueWaitIdle(context.vulkanInfo.queue_graphics);
    // 仅在等待图形队列成功，且图形与呈现所用队列不同时等待呈现队列
    if (!result && context.vulkanInfo.queue_graphics !=
                       context.vulkanInfo.queue_presentation)
        result = vkQueueWaitIdle(context.vulkanInfo.queue_presentation);
    if (result) {
        print_error("recreateSwapchain",
                    "Failed to wait for the queue to be idle! Error code:",
                    int32_t(result));
        return result;
    }
    for (auto& i : context.callbacks_destroySwapchain)
        i();
    for (auto& i : context.vulkanInfo.swapchainImageViews)
        if (i)
            vkDestroyImageView(context.vulkanInfo.device, i, nullptr);
    context.vulkanInfo.swapchainImageViews.resize(0);
    if (result = _createSwapChain_Internal())
        return result;
    for (auto& i : context.callbacks_createSwapchain)
        i();
    return VK_SUCCESS;
}
void terminateVulkan() {
    if (!context.vulkanInfo.instance)
        return;
    if (context.vulkanInfo.device) {
        vkDeviceWaitIdle(context.vulkanInfo.device);
        if (context.vulkanInfo.swapchain) {
            for (auto& i : context.callbacks_destroySwapchain)
                i();
            for (auto& i : context.vulkanInfo.swapchainImageViews)
                if (i)
                    vkDestroyImageView(context.vulkanInfo.device, i, nullptr);
            vkDestroySwapchainKHR(context.vulkanInfo.device, context.vulkanInfo.swapchain, nullptr);
        }
        vkDestroyDevice(context.vulkanInfo.device, nullptr);
    }
    if (context.vulkanInfo.surface)
        vkDestroySurfaceKHR(context.vulkanInfo.instance, context.vulkanInfo.surface, nullptr);
    if (context.vulkanInfo.debugger) {
        PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(context.vulkanInfo.instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (DestroyDebugUtilsMessenger)
            DestroyDebugUtilsMessenger(context.vulkanInfo.instance, context.vulkanInfo.debugger, nullptr);
    }
    vkDestroyInstance(context.vulkanInfo.instance, nullptr);

    context.vulkanInfo.instance = VK_NULL_HANDLE;
    context.vulkanInfo.phyDevice = VK_NULL_HANDLE;
    context.vulkanInfo.device = VK_NULL_HANDLE;
    context.vulkanInfo.surface = VK_NULL_HANDLE;
    context.vulkanInfo.swapchain = VK_NULL_HANDLE;
    context.vulkanInfo.swapchainImages.resize(0);
    context.vulkanInfo.swapchainImageViews.resize(0);
    context.vulkanInfo.swapchainCreateInfo = {};
    context.vulkanInfo.debugger = VK_NULL_HANDLE;
    return;
}
}  // namespace BL