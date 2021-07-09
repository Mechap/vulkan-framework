#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/Shader.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/graphics/ressources/DecriptorSet.hpp"
#include "renderer/graphics/ressources/DescriptorPool.hpp"
#include "renderer/sync/CommandBuffer.hpp"

DescriptorSet::DescriptorSet(std::shared_ptr<Device> _device, std::shared_ptr<DescriptorPool> _pool, std::shared_ptr<DescriptorSetLayout> _layout)
    : device(std::move(_device)), pool(std::move(_pool)), layout(std::move(_layout)) {
    auto set_layout = layout->getLayout();
    auto currentPool = pool->getPool();

    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    allocateInfo.descriptorPool = currentPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &set_layout;

    bool needReallocate = false;

    switch (vkAllocateDescriptorSets(device->getDevice(), &allocateInfo, &descriptor_set)) {
        case VK_SUCCESS:
            break;

        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            needReallocate = true;
            break;

        default:
            throw std::runtime_error("failed to allocate descriptor set!");
    }

    if (needReallocate) {
        currentPool = pool->pickPool();

        if (vkAllocateDescriptorSets(device->getDevice(), &allocateInfo, &descriptor_set) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor set!");
        }
    }
}

DescriptorSet::DescriptorSet(DescriptorSet &&other) noexcept
    : device(std::move(other.device)), pool(std::move(other.pool)), layout(std::move(other.layout)), descriptor_set(std::exchange(other.descriptor_set, nullptr)) {}

DescriptorSet &DescriptorSet::operator=(DescriptorSet &&other) noexcept {
    device = std::move(other.device);
    pool = std::move(other.pool);
    descriptor_set = std::exchange(other.descriptor_set, nullptr);

    return *this;
}

void DescriptorSet::bind(const GraphicsPipeline &pipeline, const CommandBuffer &cmd) const {
    vkCmdBindDescriptorSets(cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);
}

void DescriptorSet::update(const Buffer &buffer) const {
    for (auto binding : layout->getBindings()) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer.getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformObject);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        writeDescriptorSet.dstSet = descriptor_set;
        writeDescriptorSet.dstBinding = binding.binding;
        writeDescriptorSet.dstArrayElement = 0;

        writeDescriptorSet.descriptorCount = binding.descriptorCount;
        writeDescriptorSet.descriptorType = binding.descriptorType;

        writeDescriptorSet.pBufferInfo = &bufferInfo;
        writeDescriptorSet.pImageInfo = nullptr;
        writeDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device->getDevice(), 1, &writeDescriptorSet, 0, nullptr);
    }
}
