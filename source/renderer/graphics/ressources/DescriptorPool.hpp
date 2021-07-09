#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#include "utility.hpp"

class Device;
class DescriptorSetLayout;

class DescriptorPool final : public NoCopy, public NoMove {
    static constexpr std::uint32_t MAX_SETS_PER_POOL = 5;  // swapchain image count

  public:
    DescriptorPool(std::shared_ptr<Device> _device, const DescriptorSetLayout &layout, std::uint32_t poolSize);
    ~DescriptorPool();

    [[nodiscard]] VkDescriptorPool getPool();
    [[nodiscard]] VkDescriptorPool pickPool();

    void resetPools();

  private:
    VkDescriptorPool createPool(VkDescriptorPoolCreateFlags flags = 0);

  private:
    std::shared_ptr<Device> device;

    std::vector<VkDescriptorPool> used_pools;
    std::vector<VkDescriptorPool> free_pools;
    VkDescriptorPool current_pool{nullptr};

    std::vector<VkDescriptorPoolSize> pool_sizes;

    const std::uint32_t pool_size{0};
};
