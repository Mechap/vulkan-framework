#include <fmt/color.h>
#include <vulkan/vulkan_core.h>

#include <cmath>
#include <exception>

#include "config.hpp"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"
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
        auto commandBuffer = CommandBuffer(device, commandPool);

        auto renderPass = RenderPass(device, swapchain);

        auto graphicsPipeline = GraphicsPipeline(device, renderPass, swapchain, Vertex::getVertexInputDescription());
		graphicsPipeline.loadMeshes();

		device.upload_buffer(graphicsPipeline.getMesh(), BufferType::VERTEX_BUFFER);

        auto renderFence = Fence(device);

        auto presentSemaphore = Semaphore(device);
        auto renderSemaphore = Semaphore(device);

        std::vector<Framebuffer> framebuffers;

        for (std::uint32_t i = 0; i < swapchain.getImageViewCount(); ++i) {
            framebuffers.emplace_back(device, swapchain.getImageViews()[i], renderPass, swapchain);
        }

        uint32_t frameNumber = 0;

        while (!window.shouldClose()) {
            window.updateEvents();

            renderFence.wait(std::numeric_limits<std::uint64_t>::max());
            renderFence.reset();

            auto swapchainImageIndex = swapchain.acquireNextImage(presentSemaphore);

            commandBuffer.reset();
            commandBuffer.begin();

            VkClearValue clearValue;
            clearValue.color = {{0.f, 0.f, 0.f, 1.0f}};

            renderPass.begin(commandBuffer, framebuffers[swapchainImageIndex], clearValue);

            graphicsPipeline.bind(commandBuffer);

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(commandBuffer.getCommandBuffer(), 0, 1, &graphicsPipeline.getMesh().vertexBuffer.buffer, &offset);

            vkCmdDraw(commandBuffer.getCommandBuffer(), graphicsPipeline.getMesh().vertices.size(), 1, 0, 0);

            renderPass.end(commandBuffer);
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

            vkQueueSubmit(device.getQueue<QueueFamilyType::GRAPHICS>(), 1, &submit, renderFence.getFence());

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapchain.getSwapchain();

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &renderSemaphore.getSemaphore();

            presentInfo.pImageIndices = &swapchainImageIndex;
            vkQueuePresentKHR(device.getQueue<QueueFamilyType::PRESENT>(), &presentInfo);

            frameNumber++;
        }

        renderFence.wait(std::numeric_limits<std::uint64_t>::max());

        DeletionQueue::flush();
    } catch (const std::exception &e) {
        fmt::print(fmt::fg(fmt::color::crimson) | fmt::emphasis::bold, "[exception] : {}\n", e.what());
    }

    return 0;
}
