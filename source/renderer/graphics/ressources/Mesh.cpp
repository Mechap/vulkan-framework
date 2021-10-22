#include "renderer/graphics/ressources/Mesh.hpp"

#include "renderer/Device.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/sync/CommandBuffer.hpp"

Mesh::Mesh(DrawPrimitive _primitive, std::shared_ptr<Device> _device, std::span<const Vertex> _vertices, std::span<std::uint16_t> _indices)
    : device(std::move(_device)), primitive(_primitive), vertices(_vertices.begin(), _vertices.end()), indices(_indices.begin(), _indices.end()) {
    if (!vertices.empty()) {
		const auto &vbo = Buffer::createVertexBuffer(vertices, device);
		vertexBuffer.buffer = vbo.getBuffer();
		vertexBuffer.allocation = vbo.getAllocation();
    }
    if (!indices.empty()) {
		const auto &ibo = Buffer::createIndexBuffer(indices, device);
        indexBuffer.buffer = ibo.getBuffer();
        indexBuffer.allocation = ibo.getAllocation();
    }
}

void Mesh::bind(const CommandBuffer &cmd) const {
    if (!vertices.empty()) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd.getCommandBuffer(), 0, 1, &vertexBuffer.buffer, &offset);
    }

    if (!indices.empty()) {
        VkDeviceSize offset = 0;
        vkCmdBindIndexBuffer(cmd.getCommandBuffer(), indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT16);
    }
}

