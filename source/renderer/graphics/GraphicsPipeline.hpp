#pragma once

#include <vulkan/vulkan_core.h>

#include <array>
#include <glm/matrix.hpp>
#include <optional>
#include <span>
#include <vector>

#include "renderer/Device.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/sync/CommandPool.hpp"
#include "renderer/sync/Fence.hpp"
#include "utility.hpp"

class Device;
class Swapchain;

class CommandBuffer;
class RenderPass;

class DescriptorSetLayout;
class PushConstants;

struct VertexInputDescription {
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

enum class DrawPrimitive;

struct UniformObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class GraphicsPipeline : public NoCopy, public NoMove {
  public:
    struct PipelineInfo {
        PipelineInfo(
            std::shared_ptr<Device> _device, std::shared_ptr<Swapchain> _swapchain, std::shared_ptr<RenderPass> _render_pass,
            std::shared_ptr<DescriptorSetLayout> _descriptor_set_layout = nullptr, std::unique_ptr<VertexInputDescription> &&_input_info = nullptr)
            : device(std::move(_device)),
              swapchain(std::move(_swapchain)),
              render_pass(std::move(_render_pass)),
              input_info(std::move(_input_info)),
              descriptor_set_layout(std::move(_descriptor_set_layout)) {}

        std::shared_ptr<Device> device;
        std::shared_ptr<Swapchain> swapchain;
        std::shared_ptr<RenderPass> render_pass;

        std::unique_ptr<VertexInputDescription> input_info;

        std::shared_ptr<DescriptorSetLayout> descriptor_set_layout;
        std::shared_ptr<PushConstants> push_constants;
    };

  public:
    explicit GraphicsPipeline(PipelineInfo &&pipelineInfo);
    ~GraphicsPipeline();

    void bind(const CommandBuffer &commandBuffer) const;

    [[nodiscard]] VkPipeline getPipeline() const { return graphics_pipeline; }
    [[nodiscard]] VkPipelineLayout getPipelineLayout() const { return pipeline_layout; }

    [[nodiscard]] static Mesh defaultMeshTriangle();
    [[nodiscard]] static Mesh defaultMeshRectangle();

  private:
    [[nodiscard]] VkPipelineViewportStateCreateInfo createViewportState(const VkViewport &viewport, const VkRect2D &scissor) const;
    [[nodiscard]] VkPipelineColorBlendStateCreateInfo createColorBlendState() const;

    [[nodiscard]] VkPipelineLayoutCreateInfo createPipelineLayout(
        nostd::observer_ptr<VkDescriptorSetLayout> descriptorSetLayout = nullptr, nostd::observer_ptr<PushConstants> pushConstants = nullptr) const;

  private:
    PipelineInfo pipeline_info;

    VkPipeline graphics_pipeline = nullptr;
    VkPipelineLayout pipeline_layout = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;

    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
};
