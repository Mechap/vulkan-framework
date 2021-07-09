#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <glm/vec3.hpp>
#include <memory>
#include <span>
#include <vector>

class Device;
class CommandBuffer;

enum class DrawPrimitive;

struct VertexInputDescription;

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;

    static VertexInputDescription getVertexInputDescription();
};

class Mesh {
  public:
    struct AllocatedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

  public:
    Mesh() = default;
    Mesh(DrawPrimitive _primitive, std::shared_ptr<Device> _device, std::span<Vertex> _vertices = {}, std::span<std::uint16_t> _indices = {});

    virtual ~Mesh() = default;

    Mesh(Mesh &&) noexcept = default;
    Mesh &operator=(Mesh &&) noexcept = default;

    void draw(const CommandBuffer &cmd) const;

    std::shared_ptr<Device> device;

    AllocatedBuffer vertexBuffer;
    std::vector<Vertex> vertices;

    AllocatedBuffer indexBuffer;
    std::vector<std::uint16_t> indices;

    DrawPrimitive primitive;
};
