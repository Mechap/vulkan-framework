#pragma once

#include <vulkan/vulkan_core.h>

#include <optional>

#include "utility.hpp"

class Instance;

struct QueueFanmilyIndices final {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    constexpr bool isComplete() { return graphics_family.has_value() && present_family.has_value(); }
};

enum class QueueFamilyType { GRAPHICS, PRESENT };

class Device final : public NoCopy {
  public:
    explicit Device(const Instance &instance);
    ~Device();

    [[nodiscard]] const VkDevice &getDevice() const { return device; }
    [[nodiscard]] const VkPhysicalDevice &getPhysicalDevice() const { return physical_device; }

    template <QueueFamilyType type>
    [[nodiscard]] const VkQueue &getQueue() {
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

  private:
    VkDevice createLogicalDevice();

    VkPhysicalDevice pickPhysicalDevices(const Instance &instance);

    bool isDeviceSuitable(VkPhysicalDevice physicalDevice) const;
    bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice) const;

  private:
    VkPhysicalDevice physical_device;

    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;

    VkDevice device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    const VkSurfaceKHR &window_surface;
};

