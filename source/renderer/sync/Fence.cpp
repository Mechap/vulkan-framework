#include "renderer/sync/Fence.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"

Fence::Fence(const Device &device) : device(device) {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    } else {
    	DeletionQueue::push_function([dev = device.getDevice(), fc = fence]() { vkDestroyFence(dev, fc, nullptr); });
	}
}

Fence::~Fence() {
    // DeletionQueue::push_function([dev = device.getDevice(), fc = fence]() { vkDestroyFence(dev, fc, nullptr); });
}

void Fence::reset() { vkResetFences(device.getDevice(), 1, &fence); }
void Fence::wait(uint64_t timeout) { vkWaitForFences(device.getDevice(), 1, &fence, true, timeout); }
