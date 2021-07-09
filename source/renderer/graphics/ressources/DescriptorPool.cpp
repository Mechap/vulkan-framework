#include "renderer/graphics/ressources/DescriptorPool.hpp"

#include <fmt/format.h>

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"

DescriptorPool::DescriptorPool(std::shared_ptr<Device> _device, const DescriptorSetLayout &layout, const std::uint32_t poolSize) : device(std::move(_device)), pool_size(poolSize) {
    if (poolSize > MAX_SETS_PER_POOL) {
        throw std::runtime_error("pool size mustn't be greater than swapchain image count!");
    }

    std::unordered_map<VkDescriptorType, std::uint32_t> descriptorTypeCounts;
    const auto &bindings = layout.getBindings();

    for (auto &binding : bindings) {
        descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }

    pool_sizes.resize(descriptorTypeCounts.size());

    auto pool_size_it = pool_sizes.begin();
    for (auto &it : descriptorTypeCounts) {
        pool_size_it->type = it.first;
        pool_size_it->descriptorCount = it.second * poolSize;
        ++pool_size_it;
    }

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    for (auto binding_flags : layout.getBindingFlags()) {
        if (binding_flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) {
            descriptorPoolCreateInfo.flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
        }
    }

    descriptorPoolCreateInfo.poolSizeCount = std::uint32_t(pool_sizes.size());
    descriptorPoolCreateInfo.pPoolSizes = pool_sizes.data();
    descriptorPoolCreateInfo.maxSets = poolSize;

    current_pool = pickPool();
}

DescriptorPool::~DescriptorPool() {
    for (auto p : free_pools) {
        vkDestroyDescriptorPool(device->getDevice(), p, nullptr);
    }

    for (auto p : used_pools) {
        vkDestroyDescriptorPool(device->getDevice(), p, nullptr);
    }
}

[[nodiscard]] VkDescriptorPool DescriptorPool::getPool() {
    if (current_pool == nullptr) {
        current_pool = pickPool();
    }
    return current_pool;
}

[[nodiscard]] VkDescriptorPool DescriptorPool::pickPool() {
    if (!free_pools.empty()) {
        VkDescriptorPool pool = free_pools.back();

        free_pools.pop_back();
        used_pools.push_back(pool);
        return pool;
    } else {
        VkDescriptorPool pool = createPool();

        free_pools.pop_back();
        used_pools.push_back(pool);
        return pool;
    }
}

void DescriptorPool::resetPools() {
    for (auto p : used_pools) {
        vkResetDescriptorPool(device->getDevice(), p, 0);
    }

    free_pools = used_pools;
    used_pools.clear();

    current_pool = nullptr;
}

VkDescriptorPool DescriptorPool::createPool(VkDescriptorPoolCreateFlags flags) {
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = flags;

    poolInfo.maxSets = MAX_SETS_PER_POOL;
    poolInfo.poolSizeCount = std::uint32_t(pool_sizes.size());
    poolInfo.pPoolSizes = pool_sizes.data();

    VkDescriptorPool pool = nullptr;
    if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    free_pools.push_back(pool);

    return pool;
}
