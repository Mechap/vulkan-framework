#include "renderer/graphics/ressources/Image.hpp"

#include <vendor/stb_image.h>

#include <stdexcept>
#include <string_view>
#include <utility>

#include "renderer/Device.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/sync/CommandBuffer.hpp"
#include "renderer/sync/CommandPool.hpp"
#include "utility.hpp"

Image::Image(std::shared_ptr<Device> device, std::string_view filepath) : m_device{std::move(device)} {
    int textWidth, textHeight, textChannels;
    stbi_uc *pixels = stbi_load(filepath.data(), &textWidth, &textHeight, &textChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = textWidth * textHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    } else {
        imageWidth = textWidth;
        imageHeight = textHeight;
    }

    auto stagingBuffer = Buffer(m_device, Buffer::Type::STAGING, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void *data;
    void *pixel_ptr = pixels;

    vmaMapMemory(m_device->getAllocator(), stagingBuffer.getAllocation(), &data);
    std::memcpy(data, pixel_ptr, imageSize);
    vmaUnmapMemory(m_device->getAllocator(), stagingBuffer.getAllocation());

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<std::uint32_t>(textWidth);
    imageInfo.extent.height = static_cast<std::uint32_t>(textHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(m_device->getAllocator(), &imageInfo, &allocInfo, &image, &textureAllocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    } else {
        transitionLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copy(stagingBuffer);

        transitionLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

Image::~Image() {
    DeletionQueue::push_function([allocator = m_device->getAllocator(), img = image, textureAlloc = textureAllocation] { vmaDestroyImage(allocator, img, textureAlloc); });
}

void Image::bind() const { vmaBindImageMemory(m_device->getAllocator(), textureAllocation, image); }

void Image::copy(const Buffer &stagingBuffer) {
    const auto commandPool = CommandPool(m_device, QueueFamilyType::GRAPHICS);
    const auto cmd = CommandBuffer(*m_device, commandPool);

    cmd.begin();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        imageWidth,
        imageHeight,
        1,
    };

    vkCmdCopyBufferToImage(cmd.getCommandBuffer(), stagingBuffer.getBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    cmd.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd.getCommandBuffer();

    vkQueueSubmit(m_device->getQueue(QueueFamilyType::GRAPHICS), 1, &submitInfo, nullptr);
    vkQueueWaitIdle(m_device->getQueue(QueueFamilyType::GRAPHICS));
}

void Image::transitionLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    const auto cmdPool = CommandPool(m_device, QueueFamilyType::GRAPHICS);
    const auto cmd = CommandBuffer(*m_device, cmdPool);

    cmd.begin();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    barrier.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(cmd.getCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    cmd.end();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd.getCommandBuffer();

    vkQueueSubmit(m_device->getQueue(QueueFamilyType::GRAPHICS), 1, &submitInfo, nullptr);
    vkQueueWaitIdle(m_device->getQueue(QueueFamilyType::GRAPHICS));
}
