#pragma once

#include <vulkan/vulkan_core.h>

#include "utility.hpp"

class Device;

class Fence final : public NoCopy, public NoMove {
  public:
    explicit Fence(const Device &device);
    ~Fence();

	void reset();
	void wait(uint64_t timeout);

    const VkFence &getFence() const { return fence; }

  private:
    const Device &device;
    VkFence fence = nullptr;
};
