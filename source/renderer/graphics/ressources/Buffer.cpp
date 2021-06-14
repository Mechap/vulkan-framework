#include "renderer/graphics/ressources/Buffer.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/sync/CommandBuffer.hpp"

Buffer::Buffer(std::shared_ptr<Device> _device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
    : device(std::move(_device)), bufferSize(bufferSize) {
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
        DeletionQueue::push_function([allocator = device->getAllocator(), _buffer = buffer, _allocation = allocation]() { vmaDestroyBuffer(allocator, _buffer, _allocation); });
    }
}

Buffer Buffer::createVertexBuffer(std::span<const Vertex> vertices, const std::weak_ptr<Device> &device) {
    const VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
    if (auto dev = device.lock()) {
        auto stagingBuffer = Buffer(dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_COPY);

        void *data;
        vmaMapMemory(dev->getAllocator(), stagingBuffer.getAllocation(), &data);
        std::memcpy(data, vertices.data(), bufferSize);
        vmaUnmapMemory(dev->getAllocator(), stagingBuffer.getAllocation());

        auto vertexBuffer = Buffer(dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        copy(stagingBuffer, vertexBuffer, dev);

        return vertexBuffer;
    } else {
        throw std::runtime_error("device ptr is invalid!");
    }
}

Buffer Buffer::createIndexBuffer(std::span<std::uint16_t> indices, const std::weak_ptr<Device> &device) {
    const VkDeviceSize bufferSize = indices.size() * sizeof(std::uint16_t);
    if (auto dev = device.lock()) {
        auto stagingBuffer = Buffer(dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void *data;
        vmaMapMemory(dev->getAllocator(), stagingBuffer.getAllocation(), &data);
        std::memcpy(data, indices.data(), bufferSize);
        vmaUnmapMemory(dev->getAllocator(), stagingBuffer.getAllocation());

        auto indexBuffer = Buffer(dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        copy(stagingBuffer, indexBuffer, dev);

        return indexBuffer;
    } else {
        throw std::runtime_error("device ptr is invalid!");
    }
}

void Buffer::copy(const Buffer &src, const Buffer &dest, const std::weak_ptr<Device> &device) {
    if (auto dev = device.lock()) {
        const auto commandPool = CommandPool(dev, QueueFamilyType::GRAPHICS);
        const auto cmd = CommandBuffer(nostd::make_observer(dev.get()), commandPool);

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

        vkQueueSubmit(dev->getQueue<QueueFamilyType::GRAPHICS>(), 1, &submitInfo, nullptr);
    } else {
        throw std::runtime_error("device ptr is invalid!");
    }
}
