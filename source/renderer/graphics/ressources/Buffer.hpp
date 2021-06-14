#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <memory>

#include "utility.hpp"

class Device;
struct Vertex;

class Buffer final {
  public:
    Buffer(std::shared_ptr<Device> _device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);

    [[nodiscard]] auto getBuffer() const { return buffer; }
    [[nodiscard]] auto getAllocation() const { return allocation; }

	static Buffer createVertexBuffer(std::span<const Vertex> vertices, const std::weak_ptr<Device> &device);
    static Buffer createIndexBuffer(std::span<std::uint16_t> indices, const std::weak_ptr<Device> &device);
    static void copy(const Buffer &src, const Buffer &dest, const std::weak_ptr<Device> &device);

  private:
    std::shared_ptr<Device> device;

    VkBuffer buffer;
    VkDeviceSize bufferSize;

    VmaAllocation allocation;
};

