#include <fmt/color.h>

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

int main() {
    try {
        constexpr Vector2<uint32_t> window_size = {800, 600};

        auto window = Window(WindowSpec("application", window_size));

        auto instance = Instance(window, "application");
        auto device = Device(instance);
        auto swapchain = Swapchain(device, instance, window);

        auto commandPool = CommandPool(device, QueueFamilyType::GRAPHICS);
        auto commandBuffer = CommandBuffer(device, commandPool, 1);

        auto renderPass = RenderPass(device, swapchain, commandBuffer);
        auto graphicsPipeline = GraphicsPipeline(device, renderPass, swapchain);

        std::vector<Framebuffer> framebuffers;

        for (std::size_t i = 0; i < swapchain.getImageViewCount(); ++i) {
            framebuffers.emplace_back(Framebuffer(device, swapchain.getImageViews()[i], renderPass, window_size));
        }

        auto renderFence = Fence(device);

        auto presentSemaphore = Semaphore(device);
        auto renderSemaphore = Semaphore(device);

        uint32_t frameNumber = 0;

        while (!window.shouldClose()) {
            window.updateEvents();

            renderFence.wait(std::numeric_limits<std::uint32_t>::max());

            renderFence.reset();

            auto swapchainImageIndex = swapchain.acquireNextImage(presentSemaphore);

            commandBuffer.reset();
            commandBuffer.begin();

            VkClearValue clearValue;
            float flash = std::abs(std::sin(frameNumber / 120.f));
            clearValue.color = {{0.f, 0.f, flash, 1.0f}};

            renderPass.begin(framebuffers[swapchainImageIndex], clearValue);
            graphicsPipeline.bind(commandBuffer);

            vkCmdDraw(commandBuffer.getCommandBuffer(), 3, 1, 0, 0);

            renderPass.end();

            commandBuffer.end();

            VkSubmitInfo submit{};
            submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            submit.pWaitDstStageMask = &waitStages;

            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &presentSemaphore.getSemaphore();

            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &renderSemaphore.getSemaphore();

            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &commandBuffer.getCommandBuffer();

            vkQueueSubmit(device.getQueue(QueueFamilyType::GRAPHICS), 1, &submit, renderFence.getFence());

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain.getSwapchain();

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &renderSemaphore.getSemaphore();

            presentInfo.pImageIndices = &swapchainImageIndex;
            vkQueuePresentKHR(device.getQueue(QueueFamilyType::PRESENT), &presentInfo);

            frameNumber++;
        }

        renderFence.wait(std::numeric_limits<std::uint32_t>::max());
        DeletionQueue::flush();
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::crimson) | fmt::emphasis::bold, "[exception] : {}\n", e.what());
    }

    return 0;
}
