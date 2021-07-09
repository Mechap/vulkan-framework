#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <span>

#include "utility.hpp"

class Device;
class CommandBuffer;

struct Vertex;
struct UniformObject;

class Buffer final {
  public:
    enum class Type {
        VBO,
        IBO,
        UBO,
    };

  public:
    Buffer(std::shared_ptr<Device> _device, const Type _type, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);

    [[nodiscard]] auto getBuffer() const { return buffer; }
    [[nodiscard]] auto getAllocation() const { return allocation; }

    [[nodiscard]] Type getType() const { return type; }

    void bind(const CommandBuffer &cmd) const;
    void update(const UniformObject &ubo);

    static Buffer createVertexBuffer(std::span<const Vertex> vertices, const std::shared_ptr<Device> &device);
    static Buffer createIndexBuffer(std::span<const std::uint16_t> indices, const std::shared_ptr<Device> &device);
    static Buffer createUniformBuffer(std::uint32_t bufferSize, const std::shared_ptr<Device> &device);

    static void copy(const Buffer &src, const Buffer &dest, const std::shared_ptr<Device> &device);

  private:
    std::shared_ptr<Device> device;
    const Type type;

    VkBuffer buffer;
    VkDeviceSize bufferSize;

    VmaAllocation allocation;
};
