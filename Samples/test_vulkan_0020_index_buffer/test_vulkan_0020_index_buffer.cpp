/****************************************************************************
* test_vulkan_samples - Copyright (C) 2022 by LostPeter
* 
* Author: LostPeter
* Time:   2022-11-05
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
****************************************************************************/

//Platform
#define PLATFORM_IOS				1
#define PLATFORM_ANDROID			2
#define PLATFORM_WIN32				3
#define PLATFORM_MAC				4

#if defined(__APPLE_CC__)
    #undef  PLATFORM
	#if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 60000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
		#define PLATFORM			PLATFORM_IOS
	#else
		#define PLATFORM			PLATFORM_MAC
	#endif
#elif defined(__ANDROID__)
	#undef  PLATFORM
	#define PLATFORM				PLATFORM_ANDROID
#elif defined(WIN32) 
    #undef  PLATFORM
	#define PLATFORM				PLATFORM_WIN32
	#pragma warning (disable:4127)  
#else
	#pragma error "UnKnown platform! Abort! Abort!"
#endif

#if PLATFORM == PLATFORM_WIN32
    #undef min
	#undef max
	#if defined(__MINGW32__)
		#include <unistd.h>
	#endif
	#include <io.h>
	#include <process.h>
    #include <Windows.h>
    #include <shlwapi.h>
#elif PLATFORM == PLATFORM_ANDROID

#elif PLATFORM == PLATFORM_MAC
    #include "ocutil.h"
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <array>
#include <vector>
#include <set>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
    const bool g_enableValidationLayers = false;
#else
    const bool g_enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) 
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}

#if PLATFORM == PLATFORM_WIN32
    char* Unicode2Utf8(wchar_t* unicodeStr) {
        int cStrLen = WideCharToMultiByte(CP_UTF8, 0, unicodeStr, -1, NULL, 0, NULL, NULL);
        char* cStr = (char*)malloc(sizeof(char) * (cStrLen + 1));
        WideCharToMultiByte(CP_UTF8, 0, unicodeStr, -1, cStr, cStrLen + 1, NULL, NULL);
        *(cStr + cStrLen) = '\0';
        return cStr;
    }
#endif
std::string GetAssetsPath()
{
    std::string path;

    #if PLATFORM == PLATFORM_WIN32
        wchar_t szBuf[512];
        ::GetModuleFileNameW(NULL, szBuf, 512);
        ::PathRemoveFileSpecW(szBuf);
        char* utf8 = Unicode2Utf8(szBuf);

        path.append(utf8);
        free(utf8); 

        std::replace(path.begin(), path.end(), '\\', '/');

    #elif PLATFORM == PLATFORM_MAC
        path = OCUtil_GetPathExecute();
        
    #endif
        std::cout << "Path exe: " << path << std::endl; 
        if (path[path.size() - 1] == '.')
        {
            path = path.substr(0, path.size() - 2);
        }
        size_t index = path.find_last_of('/');
        path = path.substr(0, index);
        path += "/";

    return path;
}

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex 
{
    glm::vec2 pos;
    glm::vec3 color;

    Vertex()
    {

    }

    Vertex(const glm::vec2& pos, const glm::vec3& color)
    {
        this->pos = pos;
        this->color = color;
    }

    static VkVertexInputBindingDescription getBindingDescription() 
    {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};


class Application
{
public:
    Application()
    {
        this->validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        this->deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        //Vertex
        this->vertices.push_back(Vertex(glm::vec2(-0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f)));
        this->vertices.push_back(Vertex(glm::vec2( 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)));
        this->vertices.push_back(Vertex(glm::vec2( 0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)));
        this->vertices.push_back(Vertex(glm::vec2(-0.5f,  0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));
        //Index
        this->indices.push_back(0); this->indices.push_back(1); this->indices.push_back(2);
        this->indices.push_back(2); this->indices.push_back(3); this->indices.push_back(0);

        this->physicalDevice = nullptr;
        this->currentFrame = 0;
        this->framebufferResized = false;
    }

public:
    void run() 
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

 private:
    GLFWwindow* window;

    std::vector<const char*> validationLayers;
    std::vector<const char*> deviceExtensions;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame;

    uint32_t graphicsIndex;
    uint32_t presentIndex;
    SwapChainSupportDetails swapChainSupport;
    bool framebufferResized;

    void initWindow() 
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        this->window = glfwCreateWindow(WIDTH, HEIGHT, "LostPeter - test_vulkan_0020_index_buffer", nullptr, nullptr);
        glfwSetWindowUserPointer(this->window, this);
        glfwSetFramebufferSizeCallback(this->window, framebufferResizeCallback);

        std::cout << "Application.initWindow: success !" << std::endl;
    }
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        Application* app = (Application*)glfwGetWindowUserPointer(window);
        app->framebufferResized = true;
    }

    void initVulkan()
    {
        createInstance();
        setUpDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        createCommandBuffers();
        createSyncObjects();

        std::cout << "Application.initVulkan: success !" << std::endl;
    }
    void createInstance()
    {
        if (g_enableValidationLayers && !checkValidationLayerSupport()) 
        {
            throw std::runtime_error("validation layers requested, but not available !");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "test_vulkan_0020_index_buffer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (g_enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(this->validationLayers.size());
            createInfo.ppEnabledLayerNames = this->validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } 
        else 
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vulkan instance !");
        }
    }
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        int count = (int)this->validationLayers.size();
        for (int i = 0; i < count; i++)
        {
            const char* layerName = this->validationLayers[i];
            bool layerFound = false;

            int count_avai = (int)availableLayers.size();
            for (int j = 0; j < count_avai; j++)
            {
                VkLayerProperties& layerProperties = availableLayers[j];
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        return true;
    }
    void setUpDebugMessenger()
    {
        if (!g_enableValidationLayers)
            return;
        
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        populateDebugMessengerCreateInfo(createInfo);
            
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (g_enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    void createSurface()
    {
        VkResult result = glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface);
        if (result != VK_SUCCESS)
        {
            std::ostringstream os;
            os << (int)result;
            throw std::runtime_error("failed to create window surface, result: " + os.str());
        }
    }
    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs width Vulkan support !");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

        int count_device = (int)devices.size();
        for (int i = 0; i < count_device; i++)
        {
            VkPhysicalDevice& device = devices[i];
            int indexGraphics = -1;
            int indexPresent = -1;
            if (isDeviceSuitable(device, indexGraphics, indexPresent))
            {
                this->physicalDevice = device;
                this->graphicsIndex = indexGraphics;
                this->presentIndex = indexPresent;
                break;
            }
        } 

        if (this->physicalDevice == nullptr) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    bool isDeviceSuitable(VkPhysicalDevice device, 
                          int& indexGraphics,
                          int& indexPresent)
    {   
        findQueueFamilies(device, indexGraphics, indexPresent);
        if (indexGraphics == -1 || indexPresent == -1)
            return false;
        
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            querySwapChainSupport(device, this->swapChainSupport);
            swapChainAdequate = !this->swapChainSupport.formats.empty() && !this->swapChainSupport.presentModes.empty();
        }
        return swapChainAdequate;
    }
    void findQueueFamilies(VkPhysicalDevice device,
                          int& indexGraphics,
                          int& indexPresent)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int count_device = (int)queueFamilies.size();
        for (int i = 0; i < count_device; i++)
        {
            VkQueueFamilyProperties& queueFamily = queueFamilies[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indexGraphics = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);
            if (presentSupport)
            {
                indexPresent = i;
            }

            if (indexGraphics >= 0 && indexPresent >= 0)
                break;
        }
    }
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) 
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(this->deviceExtensions.begin(), this->deviceExtensions.end());
        int count = (int)availableExtensions.size();
        for (int i = 0; i < count; i++)
        {
            const VkExtensionProperties& extension = availableExtensions[i];
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void createLogicalDevice()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;
        uniqueQueueFamilies.insert(this->graphicsIndex);
        uniqueQueueFamilies.insert(this->presentIndex);

        float queuePriority = 1.0f;
        for (std::set<uint32_t>::iterator it = uniqueQueueFamilies.begin(); it != uniqueQueueFamilies.end(); ++it)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = *it;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = this->deviceExtensions.data();

        if (g_enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(this->validationLayers.size());
            createInfo.ppEnabledLayerNames = this->validationLayers.data();
        } 
        else 
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->device) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(this->device, this->graphicsIndex, 0, &this->graphicsQueue);
        vkGetDeviceQueue(this->device, this->presentIndex, 0, &this->presentQueue);
    }

    void createSwapChain()
    {
        querySwapChainSupport(this->physicalDevice, this->swapChainSupport);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(this->swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(this->swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(this->swapChainSupport.capabilities);

        uint32_t imageCount = this->swapChainSupport.capabilities.minImageCount + 1;
        if (this->swapChainSupport.capabilities.maxImageCount > 0 && imageCount > this->swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = this->swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] =
        {
            this->graphicsIndex,
            this->presentIndex
        };

        if (this->graphicsIndex != this->presentIndex)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = this->swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        VkResult result = vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapChain);
        if (result != VK_SUCCESS)
        {
            std::ostringstream os;
            os << (int)result;
            throw std::runtime_error("failed to create swap chain, result: " + os.str());
        }

        vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, nullptr);
        this->swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, this->swapChainImages.data());

        this->swapChainImageFormat = surfaceFormat.format;
        this->swapChainExtent = extent;
    }
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, SwapChainSupportDetails& details)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }
     VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
     {
        int count = (int)availableFormats.size();
        for (int i = 0; i < count; i++)
        {
            const VkSurfaceFormatKHR& availableFormat = availableFormats[i];
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
    {
        int count = (int)availablePresentModes.size();
        for (int i = 0; i < count; i++)
        {
            const VkPresentModeKHR& availablePresentMode = availablePresentModes[i];
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    {
        if (capabilities.currentExtent.width != UINT32_MAX) 
        {
            return capabilities.currentExtent;
        } 
        else 
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
    uint32_t clamp(uint32_t v, uint32_t min, uint32_t max)
    {
        if (v < min)
            return min;
        if (v > max)
            return max;
        return v;
    }

    void createImageViews()
    {
        int count = (int)this->swapChainImages.size();
        this->swapChainImageViews.resize(count);

        for (int i = 0; i < count; i++)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = this->swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = this->swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(this->device, &createInfo, nullptr, &this->swapChainImageViews[i]);
            if (result != VK_SUCCESS)
            {
                std::ostringstream os;
                os << (int)result;
                throw std::runtime_error("failed to create image view, result: " + os.str());
            }
        }
    }

    void createRenderPass() 
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = this->swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createGraphicsPipeline() 
    {
        std::string pathVertexShader = GetAssetsPath() + "Assets/Shader/Vulkan/pos2_color3.vert.spv";
        std::vector<char> vertShaderCode = readFile(pathVertexShader);
        std::string pathFragmentShader = GetAssetsPath() + "Assets/Shader/Vulkan/pos2_color3.frag.spv";
        std::vector<char> fragShaderCode = readFile(pathFragmentShader);

        VkShaderModule vertShaderModule = createShaderModule("VertexShader: ", vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule("FragmentShader: ", fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)this->swapChainExtent.width;
        viewport.height = (float)this->swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        VkOffset2D offset;
        offset.x = 0;
        offset.y = 0;
        scissor.offset = offset;
        scissor.extent = this->swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = this->pipelineLayout;
        pipelineInfo.renderPass = this->renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = nullptr;

        if (vkCreateGraphicsPipelines(this->device, nullptr, 1, &pipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
        vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
    }
    VkShaderModule createShaderModule(std::string info, const std::vector<char>& code) 
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module, " + info);
        }

        return shaderModule;
    }

    void createFramebuffers()
    {
        size_t count = this->swapChainImageViews.size();
        this->swapChainFramebuffers.resize(count);

        for (size_t i = 0; i < count; i++)
        {
            VkImageView attachments[] = {
                this->swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = this->renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = this->swapChainExtent.width;
            framebufferInfo.height = this->swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &this->swapChainFramebuffers[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool() 
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = this->graphicsIndex;

        if (vkCreateCommandPool(this->device, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(this->vertices[0]) * this->vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, this->vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(this->device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertexBuffer, this->vertexBufferMemory);

        copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

        vkDestroyBuffer(this->device, stagingBuffer, nullptr);
        vkFreeMemory(this->device, stagingBufferMemory, nullptr);
    }
    void createIndexBuffer() 
    {
        VkDeviceSize bufferSize = sizeof(this->indices[0]) * this->indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, this->indices.data(), (size_t) bufferSize);
        vkUnmapMemory(this->device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->indexBuffer, this->indexBufferMemory);

        copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

        vkDestroyBuffer(this->device, stagingBuffer, nullptr);
        vkFreeMemory(this->device, stagingBufferMemory, nullptr);
    }
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(this->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(this->device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(this->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(this->device, buffer, bufferMemory, 0);
    }
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = this->commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(this->device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

            VkBufferCopy copyRegion = {};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(this->graphicsQueue);

        vkFreeCommandBuffers(this->device, this->commandPool, 1, &commandBuffer);
    }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void createCommandBuffers() 
    {
        this->commandBuffers.resize(this->swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)this->commandBuffers.size();

        if (vkAllocateCommandBuffers(this->device, &allocInfo, this->commandBuffers.data()) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < this->commandBuffers.size(); i++) 
        {
            VkCommandBuffer& commandBuffer = this->commandBuffers[i];

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = this->renderPass;
            renderPassInfo.framebuffer = this->swapChainFramebuffers[i];
            VkOffset2D offset;
            offset.x = 0;
            offset.y = 0;
            renderPassInfo.renderArea.offset = offset;
            renderPassInfo.renderArea.extent = this->swapChainExtent;

            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

                VkBuffer vertexBuffers[] = { this->vertexBuffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(commandBuffer);

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void createSyncObjects() 
    {
        this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        this->imagesInFlight.resize(this->swapChainImages.size(), nullptr);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            if (vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(this->device, &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void drawFrame() 
    {
        vkWaitForFences(this->device, 1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(this->device, this->swapChain, UINT64_MAX, this->imageAvailableSemaphores[this->currentFrame], nullptr, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        if (this->imagesInFlight[imageIndex] != nullptr) 
        {
            vkWaitForFences(this->device, 1, &this->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        this->imagesInFlight[imageIndex] = this->inFlightFences[this->currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &this->commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(this->device, 1, &this->inFlightFences[this->currentFrame]);

        if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFences[this->currentFrame]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { this->swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(this->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->framebufferResized) 
        {
            this->framebufferResized = false;
            recreateSwapChain();
        } 
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    static std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }   

    void mainLoop()
    {
        while (!glfwWindowShouldClose(this->window))
        {
            glfwPollEvents();

            drawFrame();
        }

        vkDeviceWaitIdle(this->device);
    }

    void recreateSwapChain()
    {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(this->window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(this->window, &width, &height);
            glfwWaitEvents();
        }
        
        vkDeviceWaitIdle(this->device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandBuffers();

        this->imagesInFlight.resize(this->swapChainImages.size(), nullptr);
    }

    void cleanupSwapChain()
    {
        size_t count = this->swapChainFramebuffers.size();
        for (size_t i = 0; i < count; i++)
        {
            VkFramebuffer& frameBuffer = this->swapChainFramebuffers[i];
            vkDestroyFramebuffer(this->device, frameBuffer, nullptr);
        }

        vkFreeCommandBuffers(this->device, this->commandPool, (uint32_t)this->commandBuffers.size(), this->commandBuffers.data());

        vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
        vkDestroyRenderPass(this->device, this->renderPass, nullptr);

        count = this->swapChainImageViews.size();
        for (size_t i = 0; i < count; i++)
        {
            VkImageView& imageView = this->swapChainImageViews[i];
            vkDestroyImageView(this->device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(this->device, this->swapChain, nullptr);
    }

    void cleanup()
    {
        cleanupSwapChain();

        vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
        vkFreeMemory(this->device, this->indexBufferMemory, nullptr);

        vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
        vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(this->device, this->renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(this->device, this->imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(this->device, this->inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(this->device, this->commandPool, nullptr);
        
        vkDestroyDevice(this->device, nullptr);
        if (g_enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
        }
        vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
        vkDestroyInstance(this->instance, nullptr);

        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main()
{
    Application app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}