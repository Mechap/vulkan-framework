#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <fmt/color.h>

#include <stdexcept>
#include <type_traits>

#include "config.hpp"
#include "renderer/Instance.hpp"
#include "window.hpp"

namespace {
    VkResult createDebugUtilsMessengerEXT(
        VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger) {
        if (auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"); func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *pAllocator) {
        if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"); func != nullptr) {
            return func(instance, messenger, pAllocator);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            fmt::print(fmt::fg(fmt::color::crimson) | fmt::emphasis::bold, "{}\n", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }
}  // namespace

Instance::Instance(const Window &window, std::string_view app_name, uint32_t app_version) {
    instance = createInstance(app_name, config::engine_name, app_version, config::engine_version, getRequiredExtensions());
    debug_messenger = createDebugMessenger();

    glfwCreateWindowSurface(instance, window.getWindow(), nullptr, &window_surface);
}

Instance::~Instance() {
    vkDestroySurfaceKHR(instance, window_surface, nullptr);
    if constexpr (config::enable_validation_layers) {
        destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
}

VkInstance Instance::createInstance(
    std::string_view app_name, std::string_view engine_name, uint32_t app_version, uint32_t engine_version, std::vector<const char *> &&required_extensions) {
    if constexpr (config::enable_validation_layers) {
        if (!checkValidationLayerSupport()) {
            throw std::runtime_error("failed to query validation layers!");
        }
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    app_info.pApplicationName = app_name.data();
    app_info.pEngineName = engine_name.data();
    app_info.applicationVersion = app_version;
    app_info.engineVersion = engine_version;

    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    instance_info.pApplicationInfo = &app_info;

    instance_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    instance_info.ppEnabledExtensionNames = required_extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugMessenger{};
    if constexpr (config::enable_validation_layers) {
        instance_info.enabledLayerCount = static_cast<uint32_t>(config::validation_layers.size());
        instance_info.ppEnabledLayerNames = config::validation_layers.data();

        populateDebugMessenger(debugMessenger);
        instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessenger;
    } else {
        instance_info.enabledLayerCount = 0;
        instance_info.ppEnabledLayerNames = nullptr;

        instance_info.pNext = nullptr;
    }

    VkInstance vk_instance = nullptr;
    if (vkCreateInstance(&instance_info, nullptr, &vk_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    } else {
        return vk_instance;
    }
}

VkDebugUtilsMessengerEXT Instance::createDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    populateDebugMessenger(debugInfo);

    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    if (createDebugUtilsMessengerEXT(instance, &debugInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug messenger!");
    }

    return debugMessenger;
}

void Instance::populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &debug_info) {
    debug_info = {};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = debugCallback;
}

std::vector<const char *> Instance::getRequiredExtensions() {
    uint32_t extensionCount = 0;
    const char **glfwExtensions = nullptr;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + extensionCount);
    if constexpr (config::enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    for (const char *extensionName : extensions) {
        fmt::print("{}\n", extensionName);
    }

    return extensions;
}

bool Instance::checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : config::validation_layers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
            if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}
