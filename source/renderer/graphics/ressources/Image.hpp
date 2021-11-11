#pragma once

#include <vendor/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <string_view>

#include "utility.hpp"

class Device;
class Buffer;

class Image final : public NoCopy, public NoMove {
  public:
    Image(std::shared_ptr<Device> device, std::string_view filepath);
    ~Image();

    void bind() const;

    void copy(const Buffer &);
	void transitionLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    [[nodiscard]] auto getImage() const { return image; }
    [[nodiscard]] auto getImageAllocation() const { return textureAllocation; }

  private:
    std::shared_ptr<Device> m_device;

    VkImage image{nullptr};
    VmaAllocation textureAllocation{nullptr};

    std::uint32_t imageWidth, imageHeight;
};
