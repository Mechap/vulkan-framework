#pragma once

#include <vulkan/vulkan_core.h>

#include <string_view>
#include <vector>
#include <glm/vec2.hpp>

namespace config {
	static constexpr glm::vec2 window_size = {800, 600};
    static constexpr bool enable_validation_layers = true;

    static constexpr std::string_view engine_name = "mechap engine";
    static constexpr uint32_t engine_version = VK_MAKE_VERSION(1, 0, 0);

    static const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    static const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}  // namespace config
