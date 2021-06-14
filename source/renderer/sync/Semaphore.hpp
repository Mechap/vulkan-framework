#pragma once

#include <vulkan/vulkan_core.h>

#include "utility.hpp"

class Device;

class Semaphore final : public NoCopy, public NoMove {
  public:
    explicit Semaphore(nostd::not_null<Device> device);

    [[nodiscard]] const VkSemaphore &getSemaphore() const { return semaphore; }

  private:
    VkSemaphore semaphore = nullptr;
};
