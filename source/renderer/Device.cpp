#include "renderer/Device.hpp"

#include <fmt/core.h>

#include <set>
#include <stdexcept>
#include <unordered_set>

#include "config.hpp"
#include "renderer/Instance.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/sync/CommandBuffer.hpp"
#include "renderer/sync/Fence.hpp"
#include "renderer/sync/Semaphore.hpp"

Device::Device(const Instance &instance) : instance(instance), window_surface(instance.getSurface()) {
    physical_device = pickPhysicalDevices(instance);

    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    device = createLogicalDevice();
    allocator = createAllocator();
}

Device::~Device() {
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
}

VkDevice Device::createLogicalDevice() {
    auto indices = findQueueFamilies(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family.value(), indices.present_family.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        queueInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(config::device_extensions.size());
    deviceInfo.ppEnabledExtensionNames = config::device_extensions.data();

    deviceInfo.enabledLayerCount = 0;

    VkDevice vk_device = nullptr;
    if (vkCreateDevice(physical_device, &deviceInfo, nullptr, &vk_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create device!");
    }

    vkGetDeviceQueue(vk_device, indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(vk_device, indices.present_family.value(), 0, &present_queue);

    return vk_device;
}

VmaAllocator Device::createAllocator() {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physical_device;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance.getInstance();

    VmaAllocator vma_allocator = nullptr;
    if (vmaCreateAllocator(&allocatorInfo, &vma_allocator) != VK_SUCCESS) {
        throw std::runtime_error("failed to create VMA allocator!");
    }

    return vma_allocator;
}

QueueFanmilyIndices Device::findQueueFamilies(VkPhysicalDevice physicalDevice) const {
    QueueFanmilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, window_surface, &presentSupport);

        if (presentSupport) {
            indices.present_family = i;
        }

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}

void Device::upload_buffer(Mesh &mesh, BufferType type) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);

    switch (type) {
        case BufferType::VERTEX_BUFFER:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;

        case BufferType::INDEX_BUFFER:
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;

        default:
            throw std::runtime_error("unknow buffer usage!");
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &mesh.vertexBuffer.buffer, &mesh.vertexBuffer.allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer!");
    } else {
        DeletionQueue::push_function([_allocator = allocator, _buffer = mesh.vertexBuffer.buffer, _allocation = mesh.vertexBuffer.allocation]() {
            vmaDestroyBuffer(_allocator, _buffer, _allocation);
        });
    }

    void *data;
    vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);

    std::memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
}

VkPhysicalDevice Device::pickPhysicalDevices(const Instance &instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, devices.data());

    VkPhysicalDevice physicalDevice = nullptr;

    for (const auto &device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == nullptr) {
        throw std::runtime_error("failed to pick a physical device that supports vulkan!");
    }

    return physicalDevice;
}

bool Device::isDeviceSuitable(VkPhysicalDevice physicalDevice) const {
    auto indices = findQueueFamilies(physicalDevice);

    return indices.isComplete() && checkDeviceExtensionsSupport(physicalDevice);
}

bool Device::checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice) const {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::unordered_set<std::string> requiredExtensions(config::device_extensions.begin(), config::device_extensions.end());
    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
