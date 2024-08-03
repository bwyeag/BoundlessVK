#include "BL/init.hpp"
namespace BL {
Context context;
/*
 * 窗口相关函数
 */
void setWindowInit(int w, int h, bool isFpsDisplayed) {
    auto& wInfo = context.windowInfo;
    wInfo.width = w;
    wInfo.height = h;
    wInfo.isFpsDisplayed = isFpsDisplayed;
}
void setWindowTitle(const char* newTitle) {
    auto& wInfo = context.windowInfo;
    wInfo.pTitle = newTitle;
    glfwSetWindowTitle(wInfo.pWindow, newTitle);
}
void calcFps() {
    auto& wInfo = context.windowInfo;
    static double last_time = glfwGetTime();
    static double delta_time = 0.0;
    static uint32_t delta_frame = -1;
    static std::stringstream info;
    wInfo.lastTime = wInfo.currentTime;
    wInfo.currentTime = glfwGetTime();
    wInfo.deltaTime = wInfo.currentTime - wInfo.lastTime;
    if (wInfo.isFpsDisplayed) {
        delta_frame++;
        if ((delta_time = wInfo.currentTime - last_time) >=
            FPS_DISPLAY_DELTA_TIME) {
            if (!wInfo.isFullScreen) {
                info.precision(1);
                info << wInfo.pTitle << "  " << std::fixed
                     << delta_frame / delta_time << " FPS";
                glfwSetWindowTitle(wInfo.pWindow, info.str().c_str());
                info.str("");
            } else {
                glfwSetWindowTitle(wInfo.pWindow, wInfo.pTitle);
            }
            last_time = wInfo.currentTime;
            delta_frame = 0;
        }
    }
}
static void _size_callback(GLFWwindow* window, int width, int height) {
    context.windowInfo.height = height;
    context.windowInfo.width = width;
    print_log("WindowSize", "New window size(w/h):", width, height);
}
bool initWindow(const char* title, bool fullScreen, bool isResizable) {
    if (!glfwInit()) {
        print_error("InitWindow", "GLFW init failed!");
        return false;
    }
    auto& wInfo = context.windowInfo;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    wInfo.pMonitor = glfwGetPrimaryMonitor();
    wInfo.pTitle = title;
    if (fullScreen) {
        wInfo.isFullScreen = true;
        const GLFWvidmode* pMode = glfwGetVideoMode(wInfo.pMonitor);
        wInfo.width = pMode->width;
        wInfo.height = pMode->height;
        wInfo.pWindow = glfwCreateWindow(pMode->width, pMode->height, title,
                                         wInfo.pMonitor, nullptr);
    } else {
        wInfo.pWindow = glfwCreateWindow(wInfo.width, wInfo.height, title,
                                         nullptr, nullptr);
    }
    glfwSetWindowSizeCallback(wInfo.pWindow, _size_callback);
    if (!wInfo.pWindow) {
        print_error("InitWindow", "GLFW window create failed!");
        glfwTerminate();
        return false;
    }
    return true;
}
void setWindowFullSrceen() {
    auto& wInfo = context.windowInfo;
    const GLFWvidmode* pMode = glfwGetVideoMode(wInfo.pMonitor);
    glfwSetWindowMonitor(wInfo.pWindow, wInfo.pMonitor, 0, 0, pMode->width,
                         pMode->height, pMode->refreshRate);
    recreateSwapchain();
}
void setWindowWindowed(int offsetX, int offsetY, int width, int height) {
    auto& wInfo = context.windowInfo;
    const GLFWvidmode* pMode = glfwGetVideoMode(wInfo.pMonitor);
    glfwSetWindowMonitor(wInfo.pWindow, nullptr, offsetX, offsetY, width,
                         height, pMode->refreshRate);
    recreateSwapchain();
}
bool terminateWindow() {
    auto& wInfo = context.windowInfo;
    glfwDestroyWindow(wInfo.pWindow);
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
    if (_selectPhysicalDevice()) {
        return false;
    }
    VmaAllocatorCreateFlags vma_flag;
    if (_createDevice(&vma_flag)) {
        return false;
    }
    if (_initVMA(vma_flag)) {
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
    auto& info = context.vulkanInfo;
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
                 supportPresentation = VK_FALSE,
                 supportCompute = queueFamilyPropertieses[i].queueFlags &
                                  VK_QUEUE_COMPUTE_BIT;
        if (info.surface)
            if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
                    physicalDevice, i, info.surface, &supportPresentation)) {
                print_error("InitVulkan",
                            "Failed to check physical device "
                            "surface support! Error code: ",
                            int32_t(result));
                return result;
            }
        if (supportGraphics && supportCompute &&
            (!info.surface || supportPresentation)) {
            info.queueFamilyIndex_graphics = info.queueFamilyIndex_compute = i;
            if (info.surface)
                info.queueFamilyIndex_presentation = i;
            return VK_SUCCESS;
        }
    }
    return VK_RESULT_MAX_ENUM;
}
VkResult _selectPhysicalDevice() {
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
const uint32_t vma_extcheck_count = 4;
static const char* vma_extensions[vma_extcheck_count] = {
    VK_KHR_MAINTENANCE_4_EXTENSION_NAME, VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
    VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME};
static const VmaAllocatorCreateFlagBits vma_ext_flag[vma_extcheck_count] = {
    VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT,
    VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,
    VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
    VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT};
std::vector<const char*> _selectDeviceExtensions(
    const std::vector<VkExtensionProperties>& supported_ext) {
    std::vector<const char*> extensions{"VK_KHR_swapchain"};
    extensions.reserve(extensions.size() + supported_ext.size());
    for (const VkExtensionProperties& it : supported_ext) {
        for (uint32_t i = 0; i < vma_extcheck_count; i++) {
            if (std::strcmp(vma_extensions[i], it.extensionName) == 0) {
                extensions.push_back(it.extensionName);
            }
        }
    }
    return extensions;
}
std::vector<VkExtensionProperties> _getDeviceExtensions() {
    auto& info = context.vulkanInfo;
    std::vector<VkExtensionProperties> ext;
    uint32_t ext_count = 0;
    VkResult res = vkEnumerateDeviceExtensionProperties(info.phyDevice, nullptr,
                                                        &ext_count, nullptr);
    if (res != VK_SUCCESS) {
        print_error("getDeviceExtensions", "enumerate device ext failed!");
    }
    ext.resize(ext_count);
    vkEnumerateDeviceExtensionProperties(info.phyDevice, nullptr, &ext_count,
                                         ext.data());
    if constexpr (PRINT_DEVICE_EXTENSIONS) {
        print_log("Info",
                  "Supported Device Extensions list: count:", ext_count);
        for (const VkExtensionProperties& it : ext) {
            print_log("Info", '\t', it.extensionName);
        }
    }
    return ext;
}
VkResult _createDevice(VmaAllocatorCreateFlags* out_flag) {
    auto& info = context.vulkanInfo;
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
    uint32_t& qInd_graphics = info.queueFamilyIndex_graphics;
    uint32_t& qInd_compute = info.queueFamilyIndex_compute;
    uint32_t& qInd_present = info.queueFamilyIndex_presentation;
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
    vkGetPhysicalDeviceFeatures(info.phyDevice, &physicalDeviceFeatures);
    std::vector<VkExtensionProperties> supported_ext = _getDeviceExtensions();
    std::vector<const char*> deviceExtensions =
        _selectDeviceExtensions(supported_ext);
    *out_flag = _getVMAflags(deviceExtensions);
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .flags = 0,
        .queueCreateInfoCount = queueCreateInfoCount,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledExtensionCount = uint32_t(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures};
    if (VkResult result = vkCreateDevice(info.phyDevice, &deviceCreateInfo,
                                         nullptr, &info.device)) {
        print_error("InitVulkan",
                    "Failed to create a vulkan logical device! "
                    "Error code: ",
                    int32_t(result));
        return result;
    }
    if (qInd_graphics != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(info.device, qInd_graphics, 0, &info.queue_graphics);
    if (qInd_present != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(info.device, qInd_present, 0,
                         &info.queue_presentation);
    if (qInd_compute != VK_QUEUE_FAMILY_IGNORED)
        vkGetDeviceQueue(info.device, qInd_compute, 0, &info.queue_compute);
    vkGetPhysicalDeviceProperties(info.phyDevice, &info.phyDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(info.phyDevice,
                                        &info.phyDeviceMemoryProperties);
    print_log("Init", "Renderer:", info.phyDeviceProperties.deviceName);
    return VK_SUCCESS;
}
VkResult _getSurfaceFormats(std::vector<VkSurfaceFormatKHR>& formats) {
    auto& info = context.vulkanInfo;
    uint32_t surfaceFormatCount;
    if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            info.phyDevice, info.surface, &surfaceFormatCount, nullptr)) {
        print_error("getSurfaceFormats",
                    "Failed to get the count of surface "
                    "formats! Error code:",
                    int32_t(result));
        return result;
    }
    if (!surfaceFormatCount) {
        print_error("getSurfaceFormats",
                    "Failed to find any supported surface "
                    "format!");
        abort();
    }
    formats.resize(surfaceFormatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        info.phyDevice, info.surface, &surfaceFormatCount, formats.data());
    if (result)
        print_error("getSurfaceFormats",
                    "Failed to get surface formats! Error "
                    "code:",
                    int32_t(result));
    return result;
}
VkResult _createSwapchain(bool limitFrameRate,
                          VkSwapchainCreateFlagsKHR flags) {
    auto& info = context.vulkanInfo;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            info.phyDevice, info.surface, &surfaceCapabilities)) {
        print_error(
            "createSwapchain",
            "Failed to get physical device surface capabilities!Error code:",
            int32_t(result));
        return result;
    }
    auto& createInfo = info.swapchainCreateInfo;
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
            info.phyDevice, info.surface, &surfacePresentModeCount, nullptr)) {
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
            info.phyDevice, info.surface, &surfacePresentModeCount,
            surfacePresentModes.data())) {
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
                print_log("Present Mode","VK_PRESENT_MODE_MAILBOX_KHR");
                break;
            }
    switch (createInfo.presentMode)
    {
    case VK_PRESENT_MODE_FIFO_KHR:
        print_log("Present Mode","VK_PRESENT_MODE_MAILBOX_KHR");
        break;
    }
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.flags = flags;
    createInfo.surface = info.surface;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.clipped = VK_TRUE;
    if (VkResult result = _createSwapChain_Internal())
        return result;
    _iterateCallback_CreateSwapchain();
    return VK_SUCCESS;
}
VkResult _createSwapChain_Internal() {
    auto& info = context.vulkanInfo;
    auto& createInfo = info.swapchainCreateInfo;
    if (VkResult result = vkCreateSwapchainKHR(info.device, &createInfo,
                                               nullptr, &info.swapchain)) {
        print_error("createSwapchain",
                    "Failed to create a swapchain! Error "
                    "code:",
                    int32_t(result));
        return result;
    }
    uint32_t swapchainImageCount;
    if (VkResult result = vkGetSwapchainImagesKHR(
            info.device, info.swapchain, &swapchainImageCount, nullptr)) {
        print_error("createSwapchain",
                    "Failed to get the count of swapchain images! Error code:",
                    int32_t(result));
        return result;
    }
    info.swapchainImages.resize(swapchainImageCount);
    if (VkResult result = vkGetSwapchainImagesKHR(
            info.device, info.swapchain, &swapchainImageCount,
            info.swapchainImages.data())) {
        print_error(
            "createSwapchain",
            "Failed to get swapchain images! Error code:", int32_t(result));
        return result;
    }

    info.swapchainImageViews.resize(swapchainImageCount);
    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = createInfo.imageFormat,
        //.components = {},//四个成员皆为VK_COMPONENT_SWIZZLE_IDENTITY
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    for (size_t i = 0; i < swapchainImageCount; i++) {
        imageViewCreateInfo.image = info.swapchainImages[i];
        if (VkResult result =
                vkCreateImageView(info.device, &imageViewCreateInfo, nullptr,
                                  &info.swapchainImageViews[i])) {
            print_error("createSwapchain",
                        "Failed to create a swapchain image view! Error code:",
                        int32_t(result));
            return result;
        }
    }
    return VK_SUCCESS;
}
VkResult _setSurfaceFormat(
    VkSurfaceFormatKHR surfaceFormat,
    std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats) {
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
        return recreateSwapchain();
    return VK_SUCCESS;
}
VkResult recreateSwapchain() {
    auto& info = context.vulkanInfo;
    auto& createInfo = info.swapchainCreateInfo;
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        info.phyDevice, info.surface, &surfaceCapabilities);
    if (result != VK_SUCCESS) {
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
    createInfo.oldSwapchain = info.swapchain;
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
    _iterateCallback_DestroySwapchain();
    for (auto& i : info.swapchainImageViews)
        if (i)
            vkDestroyImageView(info.device, i, nullptr);
    info.swapchainImageViews.resize(0);
    result = _createSwapChain_Internal();
    if (result != VK_SUCCESS) {
        print_error("recreateSwapchain",
                    "Create swapchain failed! Code:", int32_t(result));
        return result;
    }
    _iterateCallback_CreateSwapchain();
    print_log("Info", "Window Resized!");
    return VK_SUCCESS;
}
void terminateVulkan() {
    _destroyHandles();
    _clearHandles();
    return;
}
void waitAll() {
    auto& info = context.vulkanInfo;
    if (info.device) {
        vkDeviceWaitIdle(info.device);
    }
}
void _destroyHandles() {
    auto& info = context.vulkanInfo;
    if (!info.instance)
        return;
    if (info.device) {
        vkDeviceWaitIdle(info.device);
        _terminateVMA();
        if (info.swapchain) {
            _iterateCallback_DestroySwapchain();
            for (auto& i : info.swapchainImageViews)
                if (i)
                    vkDestroyImageView(info.device, i, nullptr);
            vkDestroySwapchainKHR(info.device, info.swapchain, nullptr);
        }
        vkDestroyDevice(info.device, nullptr);
    }
    if (info.surface)
        vkDestroySurfaceKHR(info.instance, info.surface, nullptr);
    if (info.debugger) {
        PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(info.instance,
                                      "vkDestroyDebugUtilsMessengerEXT"));
        if (DestroyDebugUtilsMessenger)
            DestroyDebugUtilsMessenger(info.instance, info.debugger, nullptr);
    }
    vkDestroyInstance(info.instance, nullptr);
}
void _clearHandles() {
    auto& info = context.vulkanInfo;
    info.instance = VK_NULL_HANDLE;
    info.phyDevice = VK_NULL_HANDLE;
    info.device = VK_NULL_HANDLE;
    info.surface = VK_NULL_HANDLE;
    info.swapchain = VK_NULL_HANDLE;
    info.swapchainImages.resize(0);
    info.swapchainImageViews.resize(0);
    info.swapchainCreateInfo = {};
    info.debugger = VK_NULL_HANDLE;
}
/*
 * VMA 相关函数
 */
VmaAllocatorCreateFlags _getVMAflags(const std::vector<const char*>& ext) {
    VmaAllocatorCreateFlags flag = 0;
    for (uint32_t i = 0; i < ext.size(); i++) {
        for (uint32_t j = 0; j < vma_extcheck_count; j++) {
            if (std::strcmp(ext[i], vma_extensions[j]) == 0) {
                //flag |= vma_ext_flag[j];
                //print_log("Info", "VMA ext found:", ext[i]);
            }
        }
    }
    return flag;
}
VkResult _initVMA(VmaAllocatorCreateFlags flag) {
    auto& info = context.vulkanInfo;
    VmaAllocatorCreateInfo create_info = {};
    create_info.flags = flag;
    create_info.vulkanApiVersion = VK_API_VERSION_1_3;
    create_info.instance = info.instance;
    create_info.device = info.device;
    create_info.physicalDevice = info.phyDevice;
    VkResult res = vmaCreateAllocator(&create_info, &info.allocator);
    if (res != VK_SUCCESS) {
        print_error("InitVMA",
                    "vma allocator create failed! Error Code:", int32_t(res));
        return res;
    }
    print_log("Info", "VMA create successfull.");
    return VK_SUCCESS;
}
void _terminateVMA() {
    auto& info = context.vulkanInfo;
    vmaDestroyAllocator(info.allocator);
}
void _iterateCallback_CreateSwapchain() {
    for (std::map<int, std::function<void()> >::iterator it =
             context.callbacks_createSwapchain.begin();
         it != context.callbacks_createSwapchain.end(); ++it) {
        (it->second)();
    }
}
void _iterateCallback_DestroySwapchain() {
    for (std::map<int, std::function<void()> >::iterator it =
             context.callbacks_destroySwapchain.begin();
         it != context.callbacks_destroySwapchain.end(); ++it) {
        (it->second)();
    }
}
int addCallback_CreateSwapchain(std::function<void()> p) {
    static uint32_t id = 1;
    context.callbacks_createSwapchain.insert({id, p});
    return id++;
}
int addCallback_DestroySwapchain(std::function<void()> p) {
    static uint32_t id = 1;
    context.callbacks_destroySwapchain.insert({id, p});
    return id++;
}
void removeCallback_CreateSwapchain(int id) {
    if (id==0) return;
    auto it = context.callbacks_createSwapchain.find(id);
    if (it == context.callbacks_createSwapchain.end()) {
        print_error("Callback", "CreateSwapchain id not found:", id);
        return;
    }
    context.callbacks_createSwapchain.erase(it);
}
void removeCallback_DestroySwapchain(int id) {
    if (id==0) return;
    auto it = context.callbacks_destroySwapchain.find(id);
    if (it == context.callbacks_destroySwapchain.end()) {
        print_error("Callback", "DestroySwapchain id not found:", id);
        return;
    }
    context.callbacks_destroySwapchain.erase(it);
}
}  // namespace BL