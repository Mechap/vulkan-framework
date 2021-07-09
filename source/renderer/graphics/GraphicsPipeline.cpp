#include "renderer/graphics/GraphicsPipeline.hpp"

#include <stdexcept>

#include "renderer/Swapchain.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "renderer/graphics/Renderer.hpp"
#include "renderer/graphics/Shader.hpp"
#include "renderer/graphics/ressources/Mesh.hpp"
#include "renderer/sync/CommandBuffer.hpp"

VertexInputDescription Vertex::getVertexInputDescription() {
    VertexInputDescription description;

    VkVertexInputBindingDescription mainBinding{};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    // position attribute
    VkVertexInputAttributeDescription positionAttribute{};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);

    description.attributes.push_back(positionAttribute);

    // color attribute
    VkVertexInputAttributeDescription colorAttribute{};
    colorAttribute.binding = 0;
    colorAttribute.location = 1;
    colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttribute.offset = offsetof(Vertex, color);

    description.attributes.push_back(colorAttribute);

    return description;
}

namespace {
    VkPipelineShaderStageCreateInfo createShaderStage(const ShaderModule &shader) {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        switch (shader.getStage()) {
            case ShaderStage::VERTEX_SHADER:
                info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;

            case ShaderStage::FRAGMENT_SHADER:
                info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;

            default:
                break;
        }

        info.module = shader.getShaderModule();
        info.pName = "main";

        return info;
    }

    VkPipelineVertexInputStateCreateInfo createVertexInputState(const VertexInputDescription *inputInfo) {
        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        if (inputInfo != nullptr) {
            info.vertexAttributeDescriptionCount = inputInfo->attributes.size();
            info.pVertexAttributeDescriptions = inputInfo->attributes.data();

            info.vertexBindingDescriptionCount = inputInfo->bindings.size();
            info.pVertexBindingDescriptions = inputInfo->bindings.data();
        } else {
            info.vertexAttributeDescriptionCount = 0;
            info.pVertexAttributeDescriptions = nullptr;

            info.vertexBindingDescriptionCount = 0;
            info.pVertexBindingDescriptions = nullptr;
        }

        return info;
    }

    VkPipelineInputAssemblyStateCreateInfo createInputAssembly(VkPrimitiveTopology topology) {
        VkPipelineInputAssemblyStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        info.topology = topology;
        info.primitiveRestartEnable = VK_FALSE;

        return info;
    }

    VkPipelineRasterizationStateCreateInfo createRasterizationState(VkPolygonMode polygonMode) {
        VkPipelineRasterizationStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        info.depthClampEnable = VK_FALSE;
        info.rasterizerDiscardEnable = VK_FALSE;

        info.polygonMode = polygonMode;
        info.lineWidth = 1.0f;

        info.cullMode = VK_CULL_MODE_FRONT_BIT;
        info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        info.depthBiasEnable = VK_FALSE;
        info.depthBiasConstantFactor = 0.0f;
        info.depthBiasClamp = 0.0f;
        info.depthBiasSlopeFactor = 0.0f;

        return info;
    }

    VkPipelineMultisampleStateCreateInfo createMultisampleState() {
        VkPipelineMultisampleStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        info.sampleShadingEnable = VK_FALSE;
        info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        info.minSampleShading = 1.0f;
        info.pSampleMask = nullptr;
        info.alphaToCoverageEnable = VK_FALSE;
        info.alphaToOneEnable = VK_FALSE;

        return info;
    }

    VkPipelineColorBlendAttachmentState createColorBlendAttachmentState() {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        return colorBlendAttachment;
    }
}  // namespace

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline::PipelineInfo &&pipelineInfo) : pipeline_info(std::move(pipelineInfo)) {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = pipeline_info.swapchain->getExtent().width;
    viewport.height = pipeline_info.swapchain->getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = pipeline_info.swapchain->getExtent();

    auto viewportInfo = createViewportState(viewport, scissor);

    color_blend_attachment = createColorBlendAttachmentState();
    auto colorBlendInfo = createColorBlendState();

    // Shaders
    ShaderModule vertexShader(pipeline_info.device, "vert.spv", ShaderStage::VERTEX_SHADER);
    shader_stages.push_back(createShaderStage(vertexShader));

    ShaderModule fragmentShader(pipeline_info.device, "frag.spv", ShaderStage::FRAGMENT_SHADER);
    shader_stages.push_back(createShaderStage(fragmentShader));

    // pipeline layout
    auto descriptorSetLayout = pipeline_info.descriptor_set_layout->getLayout();
    auto pipelineLayoutInfo = createPipelineLayout(nostd::make_observer(&descriptorSetLayout));

    if (vkCreatePipelineLayout(pipeline_info.device->getDevice(), &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // vertex input and input assembly
    if (pipeline_info.input_info) {
        vertex_input_info = createVertexInputState(pipeline_info.input_info.get());
    }

    input_assembly = createInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    rasterizer = createRasterizationState(VK_POLYGON_MODE_FILL);
    multisampling = createMultisampleState();

    // Graphics Pipeline
    VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    graphicsPipelineInfo.stageCount = shader_stages.size();
    graphicsPipelineInfo.pStages = shader_stages.data();
    graphicsPipelineInfo.pVertexInputState = &vertex_input_info;
    graphicsPipelineInfo.pInputAssemblyState = &input_assembly;
    graphicsPipelineInfo.pViewportState = &viewportInfo;
    graphicsPipelineInfo.pRasterizationState = &rasterizer;
    graphicsPipelineInfo.pMultisampleState = &multisampling;
    graphicsPipelineInfo.pDepthStencilState = nullptr;
    graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
    graphicsPipelineInfo.pDynamicState = nullptr;
    graphicsPipelineInfo.layout = pipeline_layout;

    graphicsPipelineInfo.renderPass = pipeline_info.render_pass->getPass();

    graphicsPipelineInfo.subpass = 0;
    graphicsPipelineInfo.basePipelineHandle = nullptr;
    graphicsPipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(pipeline_info.device->getDevice(), nullptr, 1, &graphicsPipelineInfo, nullptr, &graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipelineLayout(pipeline_info.device->getDevice(), pipeline_layout, nullptr);
    vkDestroyPipeline(pipeline_info.device->getDevice(), graphics_pipeline, nullptr);
}

Mesh GraphicsPipeline::defaultMeshTriangle() {
    auto mesh = Mesh();

    mesh.vertices.resize(3);

    // vertex positions
    mesh.vertices[0].position = {0.5f, 0.5f, 0.0f};
    mesh.vertices[1].position = {-0.5f, 0.5f, 0.0f};
    mesh.vertices[2].position = {0.0f, -0.5f, 0.0f};

    // vertex colors
    mesh.vertices[0].color = {1.f, 0.f, 0.0f};
    mesh.vertices[1].color = {0.f, 1.f, 0.0f};
    mesh.vertices[2].color = {0.f, 0.f, 1.0f};

	mesh.primitive = DrawPrimitive::TRIANGLE;

    return mesh;
}

Mesh GraphicsPipeline::defaultMeshRectangle() {
    auto mesh = Mesh();
    mesh.vertices.resize(4);

    // vertex positions
    mesh.vertices[0].position = {400.0f, 400.0f, 0.0f};    // right bottom
    mesh.vertices[1].position = {400.0f, 200.0f, 0.0f};   // right top
    mesh.vertices[2].position = {200.0f, 400.0f, 0.0f};   // left bottom
    mesh.vertices[3].position = {200.0f, 200.0f, 0.0f};  // left top

    // vertex colors
    mesh.vertices[0].color = {1.f, 0.f, 0.0f};
    mesh.vertices[1].color = {0.f, 1.f, 0.0f};
    mesh.vertices[2].color = {0.f, 0.f, 1.0f};
    mesh.vertices[3].color = {1.f, 1.f, 0.0f};

    // index buffer
    mesh.indices.resize(6);
    mesh.indices = {0, 1, 3, 0, 2, 1};

	mesh.primitive = DrawPrimitive::RECTANGLE;

    return mesh;
}

void GraphicsPipeline::bind(const CommandBuffer &commandBuffer) const { vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline); }

VkPipelineViewportStateCreateInfo GraphicsPipeline::createViewportState(const VkViewport &viewport, const VkRect2D &scissor) const {
    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;

    return viewportInfo;
}

VkPipelineColorBlendStateCreateInfo GraphicsPipeline::createColorBlendState() const {
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &color_blend_attachment;

    colorBlendInfo.blendConstants[0] = 0.0f;
    colorBlendInfo.blendConstants[1] = 0.0f;
    colorBlendInfo.blendConstants[2] = 0.0f;
    colorBlendInfo.blendConstants[3] = 0.0f;

    return colorBlendInfo;
}

VkPipelineLayoutCreateInfo GraphicsPipeline::createPipelineLayout(nostd::observer_ptr<VkDescriptorSetLayout> descriptorSetLayout) const {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipelineLayoutInfo.flags = 0;

    if (descriptorSetLayout) {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.get();
    } else {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    return pipelineLayoutInfo;
}
