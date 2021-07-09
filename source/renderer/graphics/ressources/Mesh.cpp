#include "renderer/graphics/ressources/Mesh.hpp"

#include "renderer/Device.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/sync/CommandBuffer.hpp"

Mesh::Mesh(DrawPrimitive _primitive, std::shared_ptr<Device> _device, std::span<Vertex> _vertices, std::span<std::uint16_t> _indices)
    : device(std::move(_device)), primitive(_primitive), vertices(_vertices.begin(), _vertices.end()), indices(_indices.begin(), _indices.end()) {
    if (!vertices.empty()) {
        vertexBuffer.buffer = Buffer::createVertexBuffer(vertices, device).getBuffer();
    }
    if (!indices.empty()) {
        indexBuffer.buffer = Buffer::createIndexBuffer(indices, device).getBuffer();
    }
}

void Mesh::draw(const CommandBuffer &cmd) const {
    if (!vertices.empty()) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd.getCommandBuffer(), 0, 1, &vertexBuffer.buffer, &offset);
    }

    if (!indices.empty()) {
        VkDeviceSize offset = 0;
        vkCmdBindIndexBuffer(cmd.getCommandBuffer(), indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT16);
    }
}

