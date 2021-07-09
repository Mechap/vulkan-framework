#include "renderer/sync/Fence.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"

Fence::Fence(std::shared_ptr<Device> _device) : device(std::move(_device)) {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }
}

Fence::~Fence() { vkDestroyFence(device->getDevice(), fence, nullptr); }

void Fence::reset() { vkResetFences(device->getDevice(), 1, &fence); }
void Fence::wait(uint64_t timeout) { vkWaitForFences(device->getDevice(), 1, &fence, true, timeout); }
