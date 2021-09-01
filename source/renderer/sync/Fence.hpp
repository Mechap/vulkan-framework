#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "utility.hpp"

class Device;

class Fence final : public NoCopy, public NoMove {
  public:
    explicit Fence(std::shared_ptr<Device> _device);

    void reset();
    void wait(uint64_t timeout);

    const VkFence &getFence() const { return fence; }

  private:
    std::shared_ptr<Device> device;
    VkFence fence = nullptr;
};
