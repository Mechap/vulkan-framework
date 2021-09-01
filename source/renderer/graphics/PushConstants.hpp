#pragma once

#include <vulkan/vulkan_core.h>

#include "utility.hpp"

class PushConstants final : public NoCopy, public NoMove {
  public:
    PushConstants();
	~PushConstants();

  private:
    VkPushConstantRange push_constant;
};
