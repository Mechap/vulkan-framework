#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>

#include "utility.hpp"

class Device;
class CommandPool;

class CommandBuffer final {
  public:
    explicit CommandBuffer(const Device &device, const CommandPool &command_pool, uint32_t commandBufferCount = 1);

    void begin() const;
    void end() const { vkEndCommandBuffer(command_buffer); }
    void reset() const { vkResetCommandBuffer(command_buffer, 0); }

    const VkCommandBuffer &getCommandBuffer() const { return command_buffer; }

  private:
    VkCommandBuffer command_buffer = nullptr;
};
