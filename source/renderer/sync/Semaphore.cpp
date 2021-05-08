#include "renderer/sync/Semaphore.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"

Semaphore::Semaphore(const Device &device) : device(device) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }
}

Semaphore::~Semaphore() {
    DeletionQueue::push_function([dev = device.getDevice(), sem = semaphore]() { vkDestroySemaphore(dev, sem, nullptr); });
}

