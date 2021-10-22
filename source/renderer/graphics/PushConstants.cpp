#include "renderer/graphics/PushConstants.hpp"

namespace {
    constexpr const auto shader_stage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::VERTEX_SHADER:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::FRAGMENT_SHADER:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
    }
}  // namespace

PushConstants::PushConstants(std::uint32_t size, ShaderStage stage, std::uint32_t offset) {
}
