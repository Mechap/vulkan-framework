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
#include "renderer/sync/CommandPool.hpp"
#include "renderer/sync/Fence.hpp"
#include "renderer/sync/Semaphore.hpp"
#include "window.hpp"

struct FrameData {
    FrameData(const Device &device)
        : presentSemaphore(device), renderSemaphore(device), renderFence(device), commandPool(device, QueueFamilyType::GRAPHICS), commandBuffer(device, commandPool) {}

    Semaphore presentSemaphore, renderSemaphore;
    Fence renderFence;

    CommandPool commandPool;
    CommandBuffer commandBuffer;
};

namespace {
    constexpr std::uint32_t FRAME_OVERLAP = 2;

    FrameData &getCurrentFrame(std::span<FrameData> frames, std::uint32_t frameNumber) {
        if (frameNumber > 1 || frameNumber < 0) {
            throw std::runtime_error("frameNumber is out of bounds!");
        }
        return frames[frameNumber % FRAME_OVERLAP];
    }
}  // namespace

int main() {
    try {
        auto window = Window(WindowSpec("application", config::window_size));
        auto instance = Instance(window, "application");

        auto device = Device(instance);
        auto swapchain = Swapchain(device, instance, window);

        auto renderPass = RenderPass(device, swapchain);

		auto vertexInputDescription = Vertex::getVertexInputDescription();
        auto graphicsPipeline = GraphicsPipeline(device, renderPass, swapchain, &vertexInputDescription);
        graphicsPipeline.loadMeshes();

        device.upload_buffer<BufferType::VERTEX_BUFFER>(graphicsPipeline.getMesh());

        std::vector<Framebuffer> framebuffers;

        for (std::uint32_t i = 0; i < swapchain.getImageViewCount(); ++i) {
            framebuffers.emplace_back(device, swapchain.getImageViews()[i], renderPass, swapchain);
        }

        std::array<FrameData, FRAME_OVERLAP> frames = {FrameData(device), FrameData(device)};

        uint32_t frameNumber = 0;

        while (!window.shouldClose()) {
            window.updateEvents();

            getCurrentFrame(frames, frameNumber).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
            getCurrentFrame(frames, frameNumber).renderFence.reset();

            auto swapchainImageIndex = swapchain.acquireNextImage(getCurrentFrame(frames, frameNumber).presentSemaphore);

            getCurrentFrame(frames, frameNumber).commandBuffer.reset();
            getCurrentFrame(frames, frameNumber).commandBuffer.begin();

            VkClearValue clearValue;
            clearValue.color = {{0.f, 0.f, 0.f, 1.0f}};

            renderPass.begin(getCurrentFrame(frames, frameNumber).commandBuffer, framebuffers[swapchainImageIndex], clearValue);

            graphicsPipeline.bind(getCurrentFrame(frames, frameNumber).commandBuffer);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(getCurrentFrame(frames, frameNumber).commandBuffer.getCommandBuffer(), 0, 1, &graphicsPipeline.getMesh().vertexBuffer.buffer, &offset);

            vkCmdDraw(getCurrentFrame(frames, frameNumber).commandBuffer.getCommandBuffer(), graphicsPipeline.getMesh().vertices.size(), 1, 0, 0);

            renderPass.end(getCurrentFrame(frames, frameNumber).commandBuffer);
            getCurrentFrame(frames, frameNumber).commandBuffer.end();

            VkSubmitInfo submit{};
            submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            submit.pWaitDstStageMask = &waitStages;

            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &getCurrentFrame(frames, frameNumber).presentSemaphore.getSemaphore();

            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &getCurrentFrame(frames, frameNumber).renderSemaphore.getSemaphore();

            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &getCurrentFrame(frames, frameNumber).commandBuffer.getCommandBuffer();

            vkQueueSubmit(device.getQueue<QueueFamilyType::GRAPHICS>(), 1, &submit, getCurrentFrame(frames, frameNumber).renderFence.getFence());

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain.getSwapchain();

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &getCurrentFrame(frames, frameNumber).renderSemaphore.getSemaphore();

            presentInfo.pImageIndices = &swapchainImageIndex;
            vkQueuePresentKHR(device.getQueue<QueueFamilyType::PRESENT>(), &presentInfo);

            // fmt::print("{}\n", frameNumber);

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
