#pragma once

#include <vulkan/vulkan_core.h>

#include "utility.hpp"

class Device;
enum class QueueFamilyType;

class Swapchain;

class CommandPool : public NoCopy, public NoMove {
  public:
    CommandPool(const Device &device, QueueFamilyType type);

    const VkCommandPool &getPool() const { return command_pool; }

  private:
    const Device &device;

    VkCommandPool command_pool = nullptr;
};
