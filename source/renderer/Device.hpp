#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <optional>

#include "utility.hpp"

class Instance;

struct Mesh;

struct QueueFanmilyIndices final {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    constexpr bool isComplete() { return graphics_family.has_value() && present_family.has_value(); }
};

enum class QueueFamilyType { GRAPHICS, PRESENT };

enum class BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
};

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

class Device final : public NoCopy, public NoMove {
  public:
    explicit Device(const Instance &instance);
    ~Device();

    [[nodiscard]] const VkDevice &getDevice() const { return device; }
    [[nodiscard]] const VkPhysicalDevice &getPhysicalDevice() const { return physical_device; }

    template <QueueFamilyType type>
    [[nodiscard]] constexpr const VkQueue &getQueue() const {
        switch (type) {
            case QueueFamilyType::GRAPHICS:
                return graphics_queue;

            case QueueFamilyType::PRESENT:
                return present_queue;

            default:
                break;
        }
    }

    QueueFanmilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice) const;

    void upload_buffer(Mesh &mesh, BufferType type);

  private:
    VkDevice createLogicalDevice();
    VmaAllocator createAllocator();

    VkPhysicalDevice pickPhysicalDevices(const Instance &instance);

    bool isDeviceSuitable(VkPhysicalDevice physicalDevice) const;
    bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice) const;

  private:
    const Instance &instance;

    VkPhysicalDevice physical_device;

    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;

    VkDevice device;
    VmaAllocator allocator;

    VkQueue graphics_queue;
    VkQueue present_queue;

    const VkSurfaceKHR window_surface;
};
