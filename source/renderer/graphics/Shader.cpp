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

        auto fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}  // namespace

ShaderModule::ShaderModule(const Device &device, const std::string_view filename, ShaderType shaderType) : device(device) {
    shader_module = create(readFile(filename));
    shader_type = shaderType;
}

ShaderModule::~ShaderModule() {
    DeletionQueue::push_function([dev = device.getDevice(), sm = shader_module]() { vkDestroyShaderModule(dev, sm, nullptr); });
}

VkShaderModule ShaderModule::create(std::vector<char> &&code) {
    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    shaderModuleInfo.codeSize = code.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule = nullptr;
    if (vkCreateShaderModule(device.getDevice(), &shaderModuleInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
