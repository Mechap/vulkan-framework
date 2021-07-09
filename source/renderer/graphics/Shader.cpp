#include "renderer/graphics/Shader.hpp"

#include <fmt/format.h>

#include <fstream>
#include <stdexcept>

#include "renderer/Device.hpp"

namespace {
    std::vector<char> readFile(const std::string_view filename) {
        std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::fstream::failure(fmt::format("couldn't load spir-v file at : {}\n", filename));
        }

        auto fileSize = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}  // namespace

ShaderModule::ShaderModule(std::shared_ptr<Device> _device, const std::string_view filename, ShaderStage shaderStage) : device(std::move(_device)), shader_stage(shaderStage) {
    shader_module = create(readFile(filename));
}

ShaderModule::~ShaderModule() { vkDestroyShaderModule(device->getDevice(), shader_module, nullptr); }

VkShaderModule ShaderModule::create(std::span<const char> code) {
    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    shaderModuleInfo.codeSize = code.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule = nullptr;
    if (vkCreateShaderModule(device->getDevice(), &shaderModuleInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
