#include "renderer/graphics/Renderer.hpp"

#include <memory>

#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"
#include "renderer/graphics/Framebuffer.hpp"
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
        FrameData(const std::shared_ptr<Device> &d)
            : presentSemaphore(*d), renderSemaphore(*d), renderFence(d), commandPool(d, QueueFamilyType::GRAPHICS), commandBuffer(*d, commandPool) {}

        Semaphore presentSemaphore, renderSemaphore;
        Fence renderFence;

        CommandPool commandPool;
        CommandBuffer commandBuffer;
    };

    constexpr std::uint32_t FRAME_OVERLAP = 2;

    FrameData &getCurrentFrame(std::span<FrameData> frames, std::uint32_t frameNumber) { return frames[frameNumber % FRAME_OVERLAP]; }

    template <typename T>
    struct is_mesh : std::false_type {};

    template <>
    struct is_mesh<Mesh> : std::true_type {};
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
        renderer_info.device, renderer_info.swapchain, renderer_info.render_pass, renderer_info.descriptor_set_layout, std::move(vertexInputDescription)));

    uniform_buffers.reserve(renderer_info.swapchain->getImageViewCount());
    desciptor_sets.reserve(renderer_info.swapchain->getImageViewCount());
    framebuffers.reserve(renderer_info.swapchain->getImageViewCount());

    for (std::uint32_t i = 0; i < renderer_info.swapchain->getImageViewCount(); ++i) {
        uniform_buffers.push_back(Buffer::createUniformBuffer(sizeof(UniformObject), renderer_info.device));
        desciptor_sets.emplace_back(renderer_info.device, renderer_info.descritptor_pool, renderer_info.descriptor_set_layout);
    }
}

void Renderer::begin(DrawPrimitive mode) {
    /*
switch (mesh.primitive) {
    case DrawPrimitive::TRIANGLE:
        mesh = renderer_info.graphics_pipeline->defaultMeshTriangleVertices();
        break;

    case DrawPrimitive::RECTANGLE:
        mesh = renderer_info.graphics_pipeline->defaultMeshRectangleVertices();
        auto indexBuffer = Buffer::createIndexBuffer(mesh.indices, renderer_info.device);
        mesh.indexBuffer = {indexBuffer.getBuffer(), indexBuffer.getAllocation()};
        break;
}

auto vertexBuffer = Buffer::createVertexBuffer(mesh.vertices, renderer_info.device);
mesh.vertexBuffer = {vertexBuffer.getBuffer(), vertexBuffer.getAllocation()};
    */
}

void Renderer::draw(auto &&_mesh) {
    static_assert(std::is_same_v<std::remove_cvref_t<decltype(_mesh)>, Mesh>, "Can not render a non-mesh type.");
    meshes.emplace_back(std::forward<decltype(_mesh)>(_mesh));
}

void Renderer::end() {
    while (!renderer_info.window->shouldClose()) {
        std::array frames = {FrameData(renderer_info.device), FrameData(renderer_info.device)};
        const auto &commandBuffer = getCurrentFrame(frames, frame_number).commandBuffer;

        renderer_info.window->updateEvents();

        getCurrentFrame(frames, frame_number).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
        getCurrentFrame(frames, frame_number).renderFence.reset();

        auto swapchainImageIndex = renderer_info.swapchain->acquireNextImage(getCurrentFrame(frames, frame_number).presentSemaphore);

        // TODO: uniform_buffers[swapchainImageIndex].update()

        commandBuffer.reset();
        commandBuffer.begin();

        VkClearValue clearValue{
            .color = {{0.f, 0.f, 0.f, 1.f}},
        };
        renderer_info.render_pass->begin(commandBuffer, framebuffers[swapchainImageIndex], clearValue);

        renderer_info.graphics_pipeline->bind(commandBuffer);

        for (const auto &mesh : meshes) {
            mesh.bind(commandBuffer);
            desciptor_sets[swapchainImageIndex].bind(*renderer_info.graphics_pipeline, commandBuffer);
            desciptor_sets[swapchainImageIndex].update(uniform_buffers[swapchainImageIndex]);

			vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), static_cast<std::uint32_t>(mesh.getIndices().size()), 1, 0, 0, 0);
        }

        renderer_info.render_pass->end(commandBuffer);
        commandBuffer.end();

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit.pWaitDstStageMask = &waitStages;
    }
}
