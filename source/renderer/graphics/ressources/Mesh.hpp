#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <glm/vec3.hpp>
#include <memory>
#include <span>
#include <vector>

class Device;
class CommandBuffer;

enum class DrawPrimitive {
    TRIANGLE,
    RECTANGLE,
};


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
    Mesh(DrawPrimitive _primitive, std::shared_ptr<Device> _device, std::span<const Vertex> _vertices = {}, std::span<std::uint16_t> _indices = {});

    virtual ~Mesh() = default;

	Mesh(const Mesh &) = default;
	Mesh &operator=(const Mesh &) = default;

    Mesh(Mesh &&) noexcept = default;
    Mesh &operator=(Mesh &&) noexcept = default;

    void bind(const CommandBuffer &cmd) const;

  public:
    [[nodiscard]] const auto &getVertexBuffer() const { return vertexBuffer; }
    [[nodiscard]] auto &getVertexBuffer() { return vertexBuffer; }

    [[nodiscard]] const auto &getIndexBuffer() const { return indexBuffer; }
    [[nodiscard]] auto &getIndexBuffer() { return indexBuffer; }

    [[nodiscard]] std::span<const Vertex> getVertices() const { return vertices; }
    [[nodiscard]] std::span<Vertex> getVertices() { return vertices; }

    [[nodiscard]] std::span<const std::uint16_t> getIndices() const { return indices; }
    [[nodiscard]] std::span<std::uint16_t> getIndices() { return indices; }

    DrawPrimitive primitive;

  private:
    std::shared_ptr<Device> device;

    AllocatedBuffer vertexBuffer;
    std::vector<Vertex> vertices;

    AllocatedBuffer indexBuffer;
    std::vector<std::uint16_t> indices;
};
