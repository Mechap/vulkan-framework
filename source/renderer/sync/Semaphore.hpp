#pragma once

#include <vulkan/vulkan_core.h>

#include "utility.hpp"

class Device;

class Semaphore final : public NoCopy, public NoMove {
  public:
    explicit Semaphore(const Device &device);

    const VkSemaphore &getSemaphore() const { return semaphore; }

  private:
    const Device &device;

    VkSemaphore semaphore = nullptr;
};
