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
    vkGetDeviceQueue(vk_device, indices.transfer_family.value(), 0, &transfer_queue);

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
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transfer_family = i;
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

AllocatedBuffer Device::createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) const {
    AllocatedBuffer buffer{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferInfo.size = bufferSize;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = bufferUsage;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer!");
    } else {
        DeletionQueue::push_function([=]() { vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation); });
    }

    return buffer;
}

void Device::createVertexBuffer(Mesh &mesh) const {
	const VkDeviceSize bufferSize = sizeof(Vertex) * mesh.vertices.size();
    auto stagingBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_COPY);

    void *data;
    vmaMapMemory(allocator, stagingBuffer.allocation, &data);
    std::memcpy(data, mesh.vertices.data(), bufferSize);
    vmaUnmapMemory(allocator, stagingBuffer.allocation);

    mesh.vertexBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    copyBuffer(stagingBuffer, mesh.vertexBuffer, bufferSize);
}

void Device::copyBuffer(AllocatedBuffer &srcBuffer, AllocatedBuffer &dstBuffer, VkDeviceSize bufferSize) const {
    const auto commandPool = CommandPool(*this, QueueFamilyType::GRAPHICS);
    const auto cmd = CommandBuffer(*this, commandPool);

    cmd.begin();

    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = bufferSize;

    vkCmdCopyBuffer(cmd.getCommandBuffer(), srcBuffer.buffer, dstBuffer.buffer, 1, &copy);

    cmd.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd.getCommandBuffer();

    vkQueueSubmit(graphics_queue, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(graphics_queue);
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
