#include <fmt/color.h>
#include <vulkan/vulkan_core.h>

#include <cmath>
#include <exception>

#include "config.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/Framebuffer.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "renderer/sync/CommandBuffer.hpp"
#include "renderer/sync/Semaphore.hpp"
#include "window.hpp"

struct FrameData {
    explicit FrameData(const std::shared_ptr<Device> &device)
        : presentSemaphore(nostd::make_not_null(device.get())),
          renderSemaphore(nostd::make_not_null(device.get())),
          renderFence(device),
          commandPool(device, QueueFamilyType::GRAPHICS),
          commandBuffer(nostd::make_observer(device.get()), commandPool) {}

    Semaphore presentSemaphore, renderSemaphore;
    Fence renderFence;

    CommandPool commandPool;
    CommandBuffer commandBuffer;
};

namespace {
    constexpr std::uint32_t FRAME_OVERLAP = 2;

    FrameData &getCurrentFrame(std::span<FrameData> frames, std::uint32_t frameNumber) {
        return frames[frameNumber % FRAME_OVERLAP];
    }
}  // namespace

int main() {
    try {
        auto window = Window(WindowSpec("application", config::window_size));

        auto instance = std::make_shared<Instance>(window, "application");
        auto device = std::make_shared<Device>(instance);
        auto swapchain = std::make_shared<Swapchain>(instance, device, window);
        auto renderPass = std::make_shared<RenderPass>(device, swapchain);

        auto vertexInputDescription = Vertex::getVertexInputDescription();

        auto graphicsPipeline = std::make_shared<GraphicsPipeline>(device, swapchain, nostd::make_not_null(renderPass.get()), &vertexInputDescription);
        auto defaultMesh = graphicsPipeline->defaultMeshRectangle();

        auto vertexBuffer = Buffer::createVertexBuffer(defaultMesh.vertices, device);
        defaultMesh.vertexBuffer = {vertexBuffer.getBuffer(), vertexBuffer.getAllocation()};

        auto indexBuffer = Buffer::createIndexBuffer(defaultMesh.indices, device);
        defaultMesh.indexBuffer = {indexBuffer.getBuffer(), indexBuffer.getAllocation()};

        std::vector<std::unique_ptr<Framebuffer>> framebuffers;

        for (std::uint32_t i = 0; i < swapchain->getImageViewCount(); ++i) {
            framebuffers.emplace_back(std::make_unique<Framebuffer>(device, nostd::make_not_null(renderPass.get()), swapchain->getImageViews()[i], swapchain->getExtent()));
        }

        std::array<FrameData, FRAME_OVERLAP> frames = {FrameData(device), FrameData(device)};

        uint32_t frameNumber = 0;

        while (!window.shouldClose()) {
            auto commandBuffer = getCurrentFrame(frames, frameNumber).commandBuffer;

            window.updateEvents();

            getCurrentFrame(frames, frameNumber).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
            getCurrentFrame(frames, frameNumber).renderFence.reset();

            auto swapchainImageIndex = swapchain->acquireNextImage(getCurrentFrame(frames, frameNumber).presentSemaphore);

            commandBuffer.reset();
            commandBuffer.begin();

            VkClearValue clearValue;
            clearValue.color = {{0.f, 0.f, 0.f, 1.0f}};
            renderPass->begin(nostd::make_not_null(&commandBuffer), nostd::make_not_null(framebuffers[swapchainImageIndex].get()), clearValue);

            graphicsPipeline->bind(commandBuffer);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffer.getCommandBuffer(), 0, 1, &defaultMesh.vertexBuffer.buffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer.getCommandBuffer(), defaultMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

            // vkCmdDraw(getCurrentFrame(frames, frameNumber).commandBuffer.getCommandBuffer(), defaultMesh.vertices.size(), 1, 0, 0);
            vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), static_cast<std::uint32_t>(defaultMesh.indices.size()), 1, 0, 0, 0);

            renderPass->end(nostd::make_not_null(&commandBuffer));
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

            vkQueueSubmit(device->getQueue<QueueFamilyType::GRAPHICS>(), 1, &submit, getCurrentFrame(frames, frameNumber).renderFence.getFence());

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain->getSwapchain();

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &getCurrentFrame(frames, frameNumber).renderSemaphore.getSemaphore();

            presentInfo.pImageIndices = &swapchainImageIndex;
            vkQueuePresentKHR(device->getQueue<QueueFamilyType::PRESENT>(), &presentInfo);

            if (frameNumber < 1) {
                frameNumber++;
            } else {
                frameNumber = 0;
            }
        }

        getCurrentFrame(frames, 0).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
        getCurrentFrame(frames, 1).renderFence.wait(std::numeric_limits<std::uint64_t>::max());

        DeletionQueue::flush();
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::crimson) | fmt::emphasis::bold, "[exception] : {}\n", e.what());
    }

    return 0;
}
