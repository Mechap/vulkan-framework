#include "renderer/sync/CommandBuffer.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/sync/CommandPool.hpp"

CommandBuffer::CommandBuffer(const Device &device, const CommandPool &command_pool, uint32_t commandBufferCount) {
    VkCommandBufferAllocateInfo commandBufferInfo{};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    commandBufferInfo.commandPool = command_pool.getPool();
    commandBufferInfo.commandBufferCount = commandBufferCount;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(device.getDevice(), &commandBufferInfo, &command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command buffers!");
    }
}

void CommandBuffer::begin() const {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffers!");
    }
}
