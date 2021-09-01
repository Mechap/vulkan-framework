#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "utility.hpp"

class Device;
struct ShaderResource;

class DescriptorSetLayout final : public NoCopy, public NoMove {
  public:
    DescriptorSetLayout(std::shared_ptr<Device> _device, std::span<const ShaderResource> shader_ressources);
    ~DescriptorSetLayout();

    [[nodiscard]] VkDescriptorSetLayout getLayout() const { return descriptor_set_layout; }

    [[nodiscard]] std::span<const VkDescriptorSetLayoutBinding> getBindings() const { return bindings; }
    [[nodiscard]] std::span<const VkDescriptorBindingFlags> getBindingFlags() const { return binding_flags; }

    std::optional<VkDescriptorSetLayoutBinding> getLayoutBindings(std::uint32_t bindingIndex) const;
    std::optional<VkDescriptorSetLayoutBinding> getLayoutBindings(std::string_view name);

  private:
    std::shared_ptr<Device> device;
    VkDescriptorSetLayout descriptor_set_layout{nullptr};

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkDescriptorBindingFlags> binding_flags;

    std::unordered_map<std::uint32_t, VkDescriptorSetLayoutBinding> bindings_lookup;
    std::unordered_map<std::uint32_t, VkDescriptorBindingFlags> binding_flags_lookup;

    std::unordered_map<std::string, std::uint32_t> ressources_lookup;
};
