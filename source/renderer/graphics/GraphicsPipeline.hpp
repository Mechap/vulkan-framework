#pragma once

#include <vulkan/vulkan_core.h>

#include <array>
#include <glm/vec3.hpp>
#include <span>
#include <vector>

#include "renderer/Device.hpp"
#include "utility.hpp"

class Device;
class Swapchain;

class CommandBuffer;
class RenderPass;

struct VertexInputDescription {
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;

    static VertexInputDescription getVertexInputDescription();
};

struct Mesh {
    AllocatedBuffer vertexBuffer;
    std::vector<Vertex> vertices;
};

class GraphicsPipeline : public NoCopy, public NoMove {
  public:
    GraphicsPipeline(const Device &device, const RenderPass &renderpass, const Swapchain &swapchain, const std::optional<VertexInputDescription> &inputInfo = std::nullopt);

    void bind(const CommandBuffer &commandBuffer) const;
    void loadMeshes();

    [[nodiscard]] const Mesh &getMesh() const { return mesh; }
    [[nodiscard]] Mesh &getMesh() { return mesh; }

  private:
    VkPipelineViewportStateCreateInfo createViewportState(const VkViewport &viewport, const VkRect2D &scissor) const;
    VkPipelineColorBlendStateCreateInfo createColorBlendState() const;

    VkGraphicsPipelineCreateInfo createGraphicsPipeline(const Device &device, const RenderPass &renderpass, const Swapchain &swapchain);
    VkPipelineLayoutCreateInfo createPipelineLayout();

  private:
    const Device &device;

    Mesh mesh;

    VkPipeline graphics_pipeline = nullptr;
    VkPipelineLayout pipeline_layout = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;

    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
};
