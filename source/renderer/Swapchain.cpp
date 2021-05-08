#include "renderer/Swapchain.hpp"

#include <limits>
#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/sync/Semaphore.hpp"
#include "window.hpp"

namespace {
    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
            extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
            extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

            return extent;
        }
    }
}  // namespace

Swapchain::Swapchain(const Device &device, const Instance &instance, const Window &window) : surface(instance.getSurface()), device(device) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), surface, &properties.capabilities);

    auto surfaceFormatCount = 0u;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &surfaceFormatCount, nullptr);

    properties.formats.resize(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &surfaceFormatCount, properties.formats.data());

    auto surfacePresentModeCount = 0u;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &surfacePresentModeCount, nullptr);

    properties.present_modes.resize(surfacePresentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &surfacePresentModeCount, properties.present_modes.data());

    swapchain = create(window.getWindow());
    createViews();
}

Swapchain::~Swapchain() {
    for (const auto &view : image_views) {
        DeletionQueue::push_function([dev = device.getDevice(), v = view]() { vkDestroyImageView(dev, v, nullptr); });
    }
    DeletionQueue::push_function([dev = device.getDevice(), sc = swapchain]() { vkDestroySwapchainKHR(dev, sc, nullptr); });
}

uint32_t Swapchain::acquireNextImage(const Semaphore &presentSemaphore) const {
    uint32_t swapchainImageIndex = 0;
    vkAcquireNextImageKHR(device.getDevice(), swapchain, std::numeric_limits<std::uint32_t>::max(), presentSemaphore.getSemaphore(), nullptr, &swapchainImageIndex);

    return swapchainImageIndex;
}

VkSwapchainKHR Swapchain::create(GLFWwindow *window) {
    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(properties.formats);
    VkPresentModeKHR presentMode = choosePresentMode(properties.present_modes);
    VkExtent2D extent = chooseExtent(properties.capabilities, window);

    image_count = properties.capabilities.minImageCount + 1;
    if (properties.capabilities.maxImageCount > 0 && image_count > properties.capabilities.maxImageCount) {
        image_count = properties.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;

    swapchainInfo.minImageCount = image_count;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFanmilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice());
    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.graphics_family.value()};

    if (indices.graphics_family != indices.present_family) {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 1;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainInfo.preTransform = properties.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;

    swapchainInfo.oldSwapchain = nullptr;

    VkSwapchainKHR swapchain = nullptr;
    if (vkCreateSwapchainKHR(device.getDevice(), &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swapchain!");
    }

    vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &image_count, nullptr);
    images.resize(image_count);
    vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &image_count, images.data());

    swapchain_image_format = surfaceFormat.format;
    swapchain_extent = extent;

    return swapchain;
}

void Swapchain::createViews() {
    image_views.resize(images.size());

    for (uint32_t i = 0; i < images.size(); ++i) {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        imageViewInfo.image = images[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = swapchain_image_format;

        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.getDevice(), &imageViewInfo, nullptr, &image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}
