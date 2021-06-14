#pragma once

#include <vulkan/vulkan_core.h>

#include <array>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
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

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct Mesh {
    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indexBuffer;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

class GraphicsPipeline : public NoCopy, public NoMove {
  public:
    GraphicsPipeline(
        std::shared_ptr<Device> _device, std::shared_ptr<Swapchain> _swapchain, nostd::not_null<RenderPass> renderpass,
        const VertexInputDescription *inputInfo = nullptr);

    void bind(const CommandBuffer &commandBuffer) const;

    [[nodiscard]] Mesh defaultMeshTriangle() const;
    [[nodiscard]] Mesh defaultMeshRectangle() const;

  private:
    [[nodiscard]] VkPipelineViewportStateCreateInfo createViewportState(const VkViewport &viewport, const VkRect2D &scissor) const;
    [[nodiscard]] VkPipelineColorBlendStateCreateInfo createColorBlendState() const;

    [[nodiscard]] VkPipelineLayoutCreateInfo createPipelineLayout() const;
    [[nodiscard]] VkDescriptorSetLayoutCreateInfo createDescriptorLayout() const;

  private:
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;

    VkPipeline graphics_pipeline = nullptr;
    VkPipelineLayout pipeline_layout = nullptr;

    VkDescriptorSetLayout decriptor_layout;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;

    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
};
