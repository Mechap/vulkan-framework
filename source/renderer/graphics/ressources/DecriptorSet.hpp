#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

class Device;
class GraphicsPipeline;
class CommandBuffer;

class DescriptorPool;
class DescriptorSetLayout;

class Buffer;

struct ShaderRessource;

class DescriptorSet final {
  public:
    DescriptorSet(std::shared_ptr<Device> _device, std::shared_ptr<DescriptorPool> _pool, std::shared_ptr<DescriptorSetLayout> _layout);
    DescriptorSet(DescriptorSet &&other) noexcept;
    DescriptorSet &operator=(DescriptorSet &&other) noexcept;

    void bind(const GraphicsPipeline &pipeline, const CommandBuffer &cmd) const;
    void update(const Buffer &buffer) const;

    [[nodiscard]] VkDescriptorSet getSet() const { return descriptor_set; }

  private:
    std::shared_ptr<Device> device;
    std::shared_ptr<DescriptorPool> pool;
    std::shared_ptr<DescriptorSetLayout> layout;

    VkDescriptorSet descriptor_set{nullptr};
};
