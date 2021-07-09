#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "utility.hpp"

class Device;
enum class QueueFamilyType;

class Swapchain;

class CommandPool : public NoCopy, public NoMove {
  public:
    CommandPool(std::shared_ptr<Device> _device, QueueFamilyType type);

    void reset(VkCommandPoolResetFlags flags = 0) const;
    [[nodiscard]] VkCommandPool getPool() const { return command_pool; }

  private:
    std::shared_ptr<Device> device;
    VkCommandPool command_pool = nullptr;
};
