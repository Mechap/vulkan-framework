#include "renderer/graphics/Renderer.hpp"

#include <memory>

#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "renderer/graphics/Shader.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/graphics/ressources/DecriptorSet.hpp"
#include "renderer/graphics/ressources/DescriptorPool.hpp"
#include "renderer/sync/CommandBuffer.hpp"
#include "renderer/sync/CommandPool.hpp"
#include "renderer/sync/Fence.hpp"
#include "renderer/sync/Semaphore.hpp"
#include "window.hpp"

namespace {
    struct FrameData {
        Semaphore presentSemaphore, renderSemaphore;
        Fence renderFence;

        CommandPool commandPool;
        CommandBuffer commandBuffer;
    };

    constexpr std::uint32_t FRAME_OVERLAP = 2;

    FrameData &getCurrentFrame(std::span<FrameData> frames, std::uint32_t frameNumber) { return frames[frameNumber % FRAME_OVERLAP]; }
}  // namespace

Renderer::Renderer(std::shared_ptr<Window> _window) {
    renderer_info.window = std::move(_window);
    renderer_info.instance = std::make_shared<Instance>(*renderer_info.window, "blank title");
    renderer_info.device = std::make_shared<Device>(renderer_info.instance);
    renderer_info.swapchain = std::make_shared<Swapchain>(renderer_info.instance, renderer_info.device, *renderer_info.window);
    renderer_info.render_pass = std::make_shared<RenderPass>(renderer_info.device, renderer_info.swapchain);

    createGraphicsPipeline();
}

Renderer::~Renderer() { DeletionQueue::flush(); }

void Renderer::createGraphicsPipeline() {
    auto vertexInputDescription = std::make_unique<VertexInputDescription>(Vertex::getVertexInputDescription());

    std::vector<ShaderResource> shaderResources;
    shaderResources.emplace_back(0, ShaderResourceType::BUFFER_UNIFORM, 1, ShaderStage::VERTEX_SHADER, ShaderResourceMode::STATIC, "mvp");

    renderer_info.descriptor_set_layout = std::make_shared<DescriptorSetLayout>(renderer_info.device, shaderResources);
    renderer_info.descritptor_pool = std::make_shared<DescriptorPool>(renderer_info.device, *renderer_info.descriptor_set_layout, renderer_info.swapchain->getImageViewCount());

    renderer_info.graphics_pipeline = std::make_shared<GraphicsPipeline>(GraphicsPipeline::PipelineInfo(
        renderer_info.device, renderer_info.swapchain, renderer_info.render_pass, std::move(vertexInputDescription), renderer_info.descriptor_set_layout.get()));
}

void Renderer::begin(DrawPrimitive mode) {
    switch (mesh.primitive) {
        case DrawPrimitive::TRIANGLE:
            mesh = renderer_info.graphics_pipeline->defaultMeshTriangle();
            break;

        case DrawPrimitive::RECTANGLE:
            mesh = renderer_info.graphics_pipeline->defaultMeshRectangle();
            auto indexBuffer = Buffer::createIndexBuffer(mesh.indices, renderer_info.device);
            mesh.indexBuffer = {indexBuffer.getBuffer(), indexBuffer.getAllocation()};
            break;
    }

    auto vertexBuffer = Buffer::createVertexBuffer(mesh.vertices, renderer_info.device);
    mesh.vertexBuffer = {vertexBuffer.getBuffer(), vertexBuffer.getAllocation()};

    std::vector<Buffer> uniformBuffers;
    uniformBuffers.reserve(renderer_info.swapchain->getImageViewCount());

    std::vector<DescriptorSet> descriptorSets;
    descriptorSets.reserve(renderer_info.swapchain->getImageViewCount());

    for (std::size_t i = 0; i < renderer_info.swapchain->getImageViewCount(); ++i) {
        uniformBuffers.push_back(Buffer::createUniformBuffer(sizeof(UniformObject), renderer_info.device));
        descriptorSets.emplace_back(renderer_info.device, renderer_info.descritptor_pool, renderer_info.descriptor_set_layout);
    }
}

void Renderer::end() {
    for (auto &buffer : vbos) {
        switch (buffer.getType()) {
            case Buffer::Type::VBO:
                break;

            default:
                break;
        }
    }
}
