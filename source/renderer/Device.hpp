#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <optional>

#include "utility.hpp"

class Instance;

struct Mesh;

struct QueueFanmilyIndices final {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> transfer_family;

    [[nodiscard]] constexpr bool isComplete() const { return graphics_family.has_value() && present_family.has_value() && transfer_family.has_value(); }
};

enum class QueueFamilyType {
    GRAPHICS,
    PRESENT,
    TRANSFER,
};

class Device final : public NoCopy, public NoMove {
  public:
    explicit Device(std::shared_ptr<Instance> instance);
    ~Device();

    [[nodiscard]] const VkDevice &getDevice() const { return device; }
    [[nodiscard]] const VkPhysicalDevice &getPhysicalDevice() const { return physical_device; }

    [[nodiscard]] const VmaAllocator &getAllocator() const { return allocator; }

    template <QueueFamilyType type>
    [[nodiscard]] constexpr const VkQueue &getQueue() const {
        switch (type) {
            case QueueFamilyType::GRAPHICS:
                return graphics_queue;

            case QueueFamilyType::PRESENT:
                return present_queue;

            case QueueFamilyType::TRANSFER:
                return transfer_queue;

            default:
                break;
        }
    }

    [[nodiscard]] QueueFanmilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice) const;

  private:
    VkDevice createLogicalDevice();
    VmaAllocator createAllocator();

    VkPhysicalDevice pickPhysicalDevices();

    bool isDeviceSuitable(VkPhysicalDevice physicalDevice) const;
    static bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice);

  private:
    std::shared_ptr<Instance> instance;

    VkPhysicalDevice physical_device;

    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;

    VkDevice device;
    VmaAllocator allocator;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
};
