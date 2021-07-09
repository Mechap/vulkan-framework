#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "utility.hpp"

class Device;

enum class ShaderStage {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
};

enum class ShaderResourceType {
    BUFFER_UNIFORM,
    BUFFER_STORAGE,
    PUSH_CONSTANT,
};

enum class ShaderResourceMode {
    STATIC,
    DYNAMIC,
    UPDATE_AFTER_BIND,
};

struct ShaderResource {
    ShaderResource(std::uint32_t _binding, ShaderResourceType _type, std::uint32_t _descriptor_count, ShaderStage _stage, ShaderResourceMode _mode, std::string_view _name)
        : binding(_binding), type(_type), descriptor_count(_descriptor_count), stage(_stage), mode(_mode), name(_name) {}

    std::uint32_t binding;
    ShaderResourceType type;
    std::uint32_t descriptor_count;
    ShaderStage stage;
    ShaderResourceMode mode;

    std::string name;
};

class ShaderModule : public NoCopy, public NoMove {
  public:
    ShaderModule(std::shared_ptr<Device> _device, std::string_view filename, ShaderStage shader_stage);
    ~ShaderModule();

    [[nodiscard]] constexpr ShaderStage getStage() const { return shader_stage; }
    [[nodiscard]] VkShaderModule getShaderModule() const { return shader_module; }

  private:
    VkShaderModule create(std::span<const char> code);

  private:
    std::shared_ptr<Device> device;

    VkShaderModule shader_module = nullptr;
    ShaderStage shader_stage;
};
