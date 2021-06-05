#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <optional>

#include "utility.hpp"

class Instance;

struct Mesh;
struct Vertex;

struct QueueFanmilyIndices final {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> transfer_family;

    constexpr bool isComplete() { return graphics_family.has_value() && present_family.has_value() && transfer_family.has_value(); }
};

enum class QueueFamilyType {
    GRAPHICS,
    PRESENT,
    TRANSFER,
};

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

            case QueueFamilyType::TRANSFER:
                return transfer_queue;

            default:
                break;
        }
    }

    QueueFanmilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice) const;

    AllocatedBuffer createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) const;
    void copyBuffer(AllocatedBuffer &srcBuffer, AllocatedBuffer &dstBuffer, VkDeviceSize bufferSize) const;

    void createVertexBuffer(Mesh &mesh) const;

    template <BufferType type>
    void upload_buffer(Mesh &mesh) {
        switch (type) {
            case BufferType::VERTEX_BUFFER:
                createVertexBuffer(mesh);
                break;

			// TODO: implement index buffers
            case BufferType::INDEX_BUFFER:
				break;

            default:
                break;
        }
    }

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
    VkQueue transfer_queue;

    const VkSurfaceKHR window_surface;
};
