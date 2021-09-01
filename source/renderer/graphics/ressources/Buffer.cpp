#include "renderer/graphics/ressources/Buffer.hpp"

#include <cstring>
#include <renderer/sync/CommandBuffer.hpp>
#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/ressources/Mesh.hpp"
#include "renderer/sync/CommandBuffer.hpp"

Buffer::Buffer(std::shared_ptr<Device> _device, const Type _type, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
    : device(std::move(_device)), bufferSize(bufferSize), type(_type) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferInfo.size = bufferSize;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = bufferUsage;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = memoryUsage;

    if (vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocationInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    } else {
        DeletionQueue::push_function([allocator = device->getAllocator(), buf = buffer, alloc = allocation]() { vmaDestroyBuffer(allocator, buf, alloc); });
    }
}

void Buffer::bind(const CommandBuffer &cmd) const {
    switch (this->type) {
        case Type::VBO:
            vkCmdBindVertexBuffers(cmd.getCommandBuffer(), 0, 1, &buffer, 0);
            break;

        case Type::IBO:
			vkCmdBindIndexBuffer(cmd.getCommandBuffer(), buffer, 0, VK_INDEX_TYPE_UINT16);
            break;

        default:
            break;
    }
}

void Buffer::update(const UniformObject &ubo) {
    if (this->type != Type::UBO) {
        throw std::runtime_error("cannot update a non ubo buffer!");
    }

    void *data;
    vmaMapMemory(device->getAllocator(), allocation, &data);
    std::memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(device->getAllocator(), allocation);
}

Buffer Buffer::createVertexBuffer(std::span<const Vertex> vertices, const std::shared_ptr<Device> &device) {
    const VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
    auto stagingBuffer = Buffer(device, Type::VBO, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_COPY);

    void *data;
    vmaMapMemory(device->getAllocator(), stagingBuffer.getAllocation(), &data);
    std::memcpy(data, vertices.data(), bufferSize);
    vmaUnmapMemory(device->getAllocator(), stagingBuffer.getAllocation());

    auto vertexBuffer = Buffer(device, Type::VBO, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    copy(stagingBuffer, vertexBuffer, device);

    return vertexBuffer;
}

Buffer Buffer::createIndexBuffer(std::span<const std::uint16_t> indices, const std::shared_ptr<Device> &device) {
    const VkDeviceSize bufferSize = indices.size() * sizeof(std::uint16_t);
    auto stagingBuffer = Buffer(device, Type::IBO, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void *data;
    vmaMapMemory(device->getAllocator(), stagingBuffer.getAllocation(), &data);
    std::memcpy(data, indices.data(), bufferSize);
    vmaUnmapMemory(device->getAllocator(), stagingBuffer.getAllocation());

    auto indexBuffer = Buffer(device, Type::IBO, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    copy(stagingBuffer, indexBuffer, device);

    return indexBuffer;
}

// TODO: should I use staging buffers to transfer data to uniform buffers or make them visible from both the cpu and gpu ?
Buffer Buffer::createUniformBuffer(std::uint32_t bufferSize, const std::shared_ptr<Device> &device) {
    auto stagingBuffer = Buffer(device, Type::UBO, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    auto uniformBuffer = Buffer(device, Type::UBO, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    copy(stagingBuffer, uniformBuffer, device);

    return uniformBuffer;
}

void Buffer::copy(const Buffer &src, const Buffer &dest, const std::shared_ptr<Device> &device) {
    const auto commandPool = CommandPool(device, QueueFamilyType::GRAPHICS);
    const auto cmd = CommandBuffer(*device, commandPool);

    cmd.begin();

    VkBufferCopy bufferCopy;
    bufferCopy.dstOffset = 0;
    bufferCopy.srcOffset = 0;
    if (src.bufferSize != dest.bufferSize) {
        throw std::runtime_error("src buffer and dest buffer do not have the same size so they can't be copied!");
    } else {
        bufferCopy.size = src.bufferSize;
    }

    vkCmdCopyBuffer(cmd.getCommandBuffer(), src.getBuffer(), dest.getBuffer(), 1, &bufferCopy);

    cmd.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd.getCommandBuffer();

    vkQueueSubmit(device->getQueue(QueueFamilyType::GRAPHICS), 1, &submitInfo, nullptr);
}
