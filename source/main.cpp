#include <fmt/color.h>
#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cmath>
#include <exception>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <renderer/graphics/Shader.hpp>

#include "config.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"
#include "renderer/graphics/Framebuffer.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "renderer/graphics/Renderer.hpp"
#include "renderer/graphics/ressources/DecriptorSet.hpp"
#include "renderer/graphics/ressources/DescriptorPool.hpp"
#include "renderer/graphics/ressources/Mesh.hpp"
#include "renderer/sync/CommandBuffer.hpp"
#include "renderer/sync/Semaphore.hpp"
#include "window.hpp"

int main() {
    try {
        auto window = std::make_shared<Window>(WindowSpec("application", config::window_size));
        auto renderer = Renderer(window);

        renderer.begin();

        auto defaultVertices = GraphicsPipeline::defaultMeshRectangleVertices();
        auto defaultIndices = GraphicsPipeline::defaultMeshRectangleIndices();

        auto mesh1 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo1 = UniformObject();
        ubo1.model = glm::mat4(1.0f);
        ubo1.view = glm::mat4(1.0f);
        ubo1.proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -100.0f, 100.0f);

        renderer.draw(mesh1, ubo1);

        auto mesh2 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo2 = ubo1; 
        ubo2.model = glm::translate(ubo2.model, glm::vec3(200.f, 200.f, 0.f));

        renderer.draw(mesh2, ubo2);

        auto mesh3 = Mesh(DrawPrimitive::RECTANGLE, renderer.getInfo().device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});
        auto ubo3 = ubo1; 
        ubo3.model = glm::translate(ubo2.model, glm::vec3(-100.f, -100.f, 0.f));

		renderer.draw(mesh3, ubo3);

        renderer.end();
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold, "[exception] : {}\n", e.what());
    }

    return 0;
}

/*
struct FrameData {
    explicit FrameData(const std::shared_ptr<Device> &device)
        : presentSemaphore(*device), renderSemaphore(*device), renderFence(device), commandPool(device, QueueFamilyType::GRAPHICS), commandBuffer(*device, commandPool) {}

    Semaphore presentSemaphore, renderSemaphore;
    Fence renderFence;

    CommandPool commandPool;
    CommandBuffer commandBuffer;
};

namespace {
    constexpr std::uint32_t FRAME_OVERLAP = 2;

    FrameData &getCurrentFrame(std::span<FrameData> frames, std::uint32_t frameNumber) { return frames[frameNumber % FRAME_OVERLAP]; }
}  // namespace

int main() {
    try {
        auto window = Window(WindowSpec("application", config::window_size));

        auto instance = std::make_shared<Instance>(window, "application");
        auto device = std::make_shared<Device>(instance);
        auto swapchain = std::make_shared<Swapchain>(instance, device, window);
        auto renderPass = std::make_shared<RenderPass>(device, swapchain);

        // graphics pipeline
        auto vertexInputDescription = std::make_unique<VertexInputDescription>(Vertex::getVertexInputDescription());

        std::vector<ShaderResource> shaderResssources;
        shaderResssources.emplace_back(0, ShaderResourceType::BUFFER_UNIFORM, 1, ShaderStage::VERTEX_SHADER, ShaderResourceMode::STATIC, "mvp");

        auto descriptorSetLayout = std::make_shared<DescriptorSetLayout>(device, shaderResssources);
        auto descriptorPool = std::make_shared<DescriptorPool>(device, *descriptorSetLayout, swapchain->getImageViewCount());

        auto graphicsPipeline =
            std::make_shared<GraphicsPipeline>(GraphicsPipeline::PipelineInfo(device, swapchain, renderPass, descriptorSetLayout, std::move(vertexInputDescription)));

        auto defaultVertices = GraphicsPipeline::defaultMeshRectangleVertices();
        auto defaultIndices = GraphicsPipeline::defaultMeshRectangleIndices();

        auto defaultMesh = Mesh(DrawPrimitive::RECTANGLE, device, {defaultVertices.begin(), defaultVertices.end()}, {defaultIndices.begin(), defaultIndices.end()});

        std::vector<Buffer> uniformBuffers;
        uniformBuffers.reserve(swapchain->getImageViewCount());

        std::vector<DescriptorSet> descriptorSets;
        descriptorSets.reserve(swapchain->getImageViewCount());

        for (std::size_t i = 0; i < swapchain->getImageViewCount(); ++i) {
            uniformBuffers.push_back(Buffer::createUniformBuffer(sizeof(UniformObject), device));
            descriptorSets.emplace_back(device, descriptorPool, descriptorSetLayout);
        }

        // framebuffers
        std::vector<std::unique_ptr<Framebuffer>> framebuffers;
        framebuffers.reserve(swapchain->getImageViewCount());

        for (std::uint32_t i = 0; i < swapchain->getImageViewCount(); ++i) {
            framebuffers.push_back(std::make_unique<Framebuffer>(device, *renderPass, swapchain->getImageViews()[i], swapchain->getExtent()));
        }

        std::array<FrameData, FRAME_OVERLAP> frames = {FrameData(device), FrameData(device)};

        uint32_t frameNumber = 0;

        auto ubo = UniformObject();
        ubo.model = glm::mat4(1.0f);
        ubo.view = glm::mat4(1.0f);
        ubo.proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -100.0f, 100.0f);

        while (!window.shouldClose()) {
            const auto &commandBuffer = getCurrentFrame(frames, frameNumber).commandBuffer;

            window.updateEvents();

            getCurrentFrame(frames, frameNumber).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
            getCurrentFrame(frames, frameNumber).renderFence.reset();

            auto swapchainImageIndex = swapchain->acquireNextImage(getCurrentFrame(frames, frameNumber).presentSemaphore);

            uniformBuffers[swapchainImageIndex].update(ubo);

            commandBuffer.reset();
            commandBuffer.begin();

            VkClearValue clearValue;
            clearValue.color = {{0.f, 0.f, 0.f, 1.0f}};
            renderPass->begin(commandBuffer, *framebuffers[swapchainImageIndex], clearValue);

            graphicsPipeline->bind(commandBuffer);

            defaultMesh.bind(commandBuffer);

            descriptorSets[swapchainImageIndex].bind(*graphicsPipeline, commandBuffer);
            descriptorSets[swapchainImageIndex].update(uniformBuffers[swapchainImageIndex]);

            vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), static_cast<std::uint32_t>(defaultMesh.getIndices().size()), 1, 0, 0, 0);

            ubo.model = glm::translate(ubo.model, glm::vec3(0.1f, 0.1f, 0.f));
            uniformBuffers[swapchainImageIndex].update(ubo);

            renderPass->end(commandBuffer);
            commandBuffer.end();

            VkSubmitInfo submit{};
            submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            submit.pWaitDstStageMask = &waitStages;

            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &getCurrentFrame(frames, frameNumber).presentSemaphore.getSemaphore();

            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &getCurrentFrame(frames, frameNumber).renderSemaphore.getSemaphore();

            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &commandBuffer.getCommandBuffer();

            vkQueueSubmit(device->getQueue(QueueFamilyType::GRAPHICS), 1, &submit, getCurrentFrame(frames, frameNumber).renderFence.getFence());

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain->getSwapchain();

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &getCurrentFrame(frames, frameNumber).renderSemaphore.getSemaphore();

            presentInfo.pImageIndices = &swapchainImageIndex;
            vkQueuePresentKHR(device->getQueue(QueueFamilyType::PRESENT), &presentInfo);

            if (frameNumber < 1) {
                frameNumber++;
            } else {
                frameNumber = 0;
            }
        }

        getCurrentFrame(frames, 0).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
        getCurrentFrame(frames, 1).renderFence.wait(std::numeric_limits<std::uint64_t>::max());

        fmt::print("instance use count : {}\n", instance.use_count());
        fmt::print("device use count : {}\n", device.use_count());
        fmt::print("swapchain use count : {}\n", swapchain.use_count());
        fmt::print("render pass use count : {}\n", renderPass.use_count());
        fmt::print("graphics pipeline use count : {}\n", graphicsPipeline.use_count());

        fmt::print("swapchain image count : {}\n", swapchain->getImageViewCount());
        fmt::print("swapchain extent : {} {}\n", swapchain->getExtent().width, swapchain->getExtent().height);

        DeletionQueue::flush();
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold, "[exception] : {}\n", e.what());
    }

    return 0;
}
*/
