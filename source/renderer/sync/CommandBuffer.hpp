#pragma once

#include <vulkan/vulkan_core.h>

#include "utility.hpp"

class Device;
class CommandPool;

class CommandBuffer final : public NoCopy, public NoMove {
  public:
    explicit CommandBuffer(const Device &device, const CommandPool &command_pool, uint32_t commandBufferCount);

    void begin();
    void end();
    void reset();

    const VkCommandBuffer &getCommandBuffer() const { return command_buffer; }

  private:
    VkCommandBuffer command_buffer = nullptr;
};
