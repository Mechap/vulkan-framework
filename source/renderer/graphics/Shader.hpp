#pragma once

#include <vulkan/vulkan_core.h>

#include <string_view>
#include <vector>

#include "utility.hpp"

class Device;

enum class ShaderType {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
};

class ShaderModule : public NoCopy, public NoMove {
  public:
    ShaderModule(const Device &device, const std::string_view filename, ShaderType shaderType);

    [[nodiscard]] constexpr ShaderType getType() const { return shader_type; }
    [[nodiscard]] const VkShaderModule &getShaderModule() const { return shader_module; }

  private:
    VkShaderModule create(std::vector<char> &&code);

  private:
    const Device &device;

    VkShaderModule shader_module = nullptr;
	ShaderType shader_type;
};

