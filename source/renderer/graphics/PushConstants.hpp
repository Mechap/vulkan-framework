#pragma once

#include <vulkan/vulkan_core.h>

#include "renderer/graphics/Shader.hpp"
#include "utility.hpp"

class PushConstants final : public NoCopy, public NoMove {
  public:
    PushConstants(std::uint32_t size, ShaderStage shaderStage, std::uint32_t offset = 0);
    ~PushConstants();

  private:
    VkPushConstantRange push_constant;
};
