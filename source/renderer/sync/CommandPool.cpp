#include "renderer/sync/CommandPool.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"

CommandPool::CommandPool(const Device &device, const QueueFamilyType type) : device(device) {
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	switch (type) {
		case QueueFamilyType::GRAPHICS:
			commandPoolInfo.queueFamilyIndex = device.findQueueFamilies(device.getPhysicalDevice()).graphics_family.value();
			break;

		case QueueFamilyType::PRESENT:
			commandPoolInfo.queueFamilyIndex = device.findQueueFamilies(device.getPhysicalDevice()).present_family.value();
			break;

		default:
			throw std::runtime_error("command pool got an incorect queue family type!");
	}

    commandPoolInfo.flags = 0;

    if (vkCreateCommandPool(device.getDevice(), &commandPoolInfo, nullptr, &command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
    }
}

CommandPool::~CommandPool() { vkDestroyCommandPool(device.getDevice(), command_pool, nullptr); }
