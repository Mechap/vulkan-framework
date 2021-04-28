#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "math/Vector2.hpp"
#include "utility.hpp"

class Device;
class RenderPass;
class Swapchain;

class GraphicsPipeline : public NoCopy, public NoMove {
  public:
    GraphicsPipeline(const Device &device, const RenderPass &renderpass, const Swapchain &swapchain, const Vector2u &window_size = {800, 600});
    ~GraphicsPipeline();

  private:
    VkPipelineViewportStateCreateInfo createViewportState(const VkViewport &viewport, const VkRect2D &scissor) const;
    VkPipelineColorBlendStateCreateInfo createColorBlendState() const;

    static VkPipelineLayoutCreateInfo createPipeline();

  private:
    const Device &device;

    VkPipeline graphics_pipeline = nullptr;
    VkPipelineLayout pipeline_layout = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;

    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
};
