#include "renderer/graphics/DescriptorSetLayout.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/graphics/Shader.hpp"

namespace {
    constexpr VkShaderStageFlags getShaderStagesFlags(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::VERTEX_SHADER:
                return VK_SHADER_STAGE_VERTEX_BIT;

            case ShaderStage::FRAGMENT_SHADER:
                return VK_SHADER_STAGE_FRAGMENT_BIT;

            default:
                throw std::runtime_error("unknown shader type!");
        }
    }

    constexpr VkDescriptorType getDescriptorType(ShaderResourceType type, bool dynamic) {
        switch (type) {
            case ShaderResourceType::BUFFER_UNIFORM:
                if (dynamic) {
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                } else {
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                }

            case ShaderResourceType::BUFFER_STORAGE:
                if (dynamic) {
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                } else {
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }

            default:
                throw std::runtime_error("Shader Ressource Type not implemented yet...");
        }
    }
}  // namespace util

DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> _device, std::span<const ShaderResource> shader_ressources) : device(std::move(_device)) {
    for (auto &ressource : shader_ressources) {
        if (ressource.mode == ShaderResourceMode::UPDATE_AFTER_BIND) {
            binding_flags.emplace_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
        } else {
            binding_flags.emplace_back(0);
        }

        VkDescriptorSetLayoutBinding layout_binding{};

        layout_binding.binding = ressource.binding;
        layout_binding.descriptorCount = ressource.descriptor_count;
        layout_binding.stageFlags = getShaderStagesFlags(ressource.stage);
        layout_binding.descriptorType = getDescriptorType(ressource.type, ressource.mode == ShaderResourceMode::DYNAMIC);

        bindings.push_back(layout_binding);

        bindings_lookup.emplace(ressource.binding, layout_binding);
        binding_flags_lookup.emplace(ressource.binding, binding_flags.back());

        ressources_lookup.emplace(ressource.name, ressource.binding);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    descriptor_set_layout_info.bindingCount = bindings.size();
    descriptor_set_layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device->getDevice(), &descriptor_set_layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

DescriptorSetLayout::~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(device->getDevice(), descriptor_set_layout, nullptr); }

std::unique_ptr<VkDescriptorSetLayoutBinding> DescriptorSetLayout::getLayoutBindings(std::uint32_t bindingIndex) const {
    auto it = bindings_lookup.find(bindingIndex);

    if (it == bindings_lookup.end()) {
        return nullptr;
    } else {
        return std::make_unique<VkDescriptorSetLayoutBinding>(it->second);
    }
}

std::unique_ptr<VkDescriptorSetLayoutBinding> DescriptorSetLayout::getLayoutBindings(std::string_view name) {
    auto it = ressources_lookup.find(name.data());

    if (it == ressources_lookup.end()) {
        return nullptr;
    } else {
        return getLayoutBindings(it->second);
    }
}
