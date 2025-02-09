
#include "graphic_pipeline.hpp"


#include <cstddef>

#include "commandBuffer/command_buffer.hpp"
#include "structs_vk.hpp"
#include "utils.hpp"

namespace TTe {

GraphicPipeline::GraphicPipeline(Device* device, GraphicPipelineCreateInfo& pipelineCreateInfo) : device(device) {
    setPipelineStage(pipelineCreateInfo);
    createShaders(pipelineCreateInfo);
    createPipelineLayout(pipelineCreateInfo);
    createVertexShaderInfo();
}

GraphicPipeline::~GraphicPipeline() {
    // shadersMap.clear();
    if(pipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
}

GraphicPipeline::GraphicPipeline(GraphicPipeline&& other) {
    vertexInputBinding = other.vertexInputBinding;
    vertexAttributes = std::move(other.vertexAttributes);
    
    shadersMap = std::move(other.shadersMap);

    pipelineStageFlags = other.pipelineStageFlags;
    pipelineDescriptorsSetsLayout = std::move(other.pipelineDescriptorsSetsLayout);
    pipelineDescriptorsSetsLayoutList = std::move(other.pipelineDescriptorsSetsLayoutList);
    pipelineLayout = other.pipelineLayout;
    pushConstantInfo = other.pushConstantInfo;

    device = other.device;
    other.pipelineLayout = VK_NULL_HANDLE;
}

GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) {
    if (this != &other) {
        vertexInputBinding = other.vertexInputBinding;
        vertexAttributes = std::move(other.vertexAttributes);
        
        shadersMap = std::move(other.shadersMap);

        pipelineStageFlags = other.pipelineStageFlags;
        pipelineDescriptorsSetsLayout = std::move(other.pipelineDescriptorsSetsLayout);
        pipelineDescriptorsSetsLayoutList = std::move(other.pipelineDescriptorsSetsLayoutList);
        pipelineLayout = other.pipelineLayout;
        pushConstantInfo = other.pushConstantInfo;

        device = other.device;
        other.pipelineLayout = VK_NULL_HANDLE;
    }
    return *this;
}

void GraphicPipeline::bindPipeline(const CommandBuffer& cmdBuffer) {
    std::vector<VkShaderEXT> shaders;
    std::vector<VkShaderStageFlagBits> shaderFlags;

    for (auto& shader : shadersMap) {
        shaders.push_back(shader.second);
        shaderFlags.push_back(shader.first);
    }
    vkCmdBindShadersEXT(cmdBuffer, shaders.size(), shaderFlags.data(), shaders.data());

    setVextexInfo(cmdBuffer);
    setRasterizerInfo(cmdBuffer);
    // setFragmentInfo(cmdBuffer);
}

void GraphicPipeline::setVextexInfo(VkCommandBuffer cmdBuffer) {
    vkCmdSetVertexInputEXT(cmdBuffer, 1, &vertexInputBinding, vertexAttributes.size(), vertexAttributes.data());
    vkCmdSetPrimitiveTopology(cmdBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdSetPrimitiveRestartEnable(cmdBuffer, VK_FALSE);
}

void GraphicPipeline::setRasterizerInfo(VkCommandBuffer cmdBuffer) {
    vkCmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    uint32_t sampleMask = 0xFF;
    vkCmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vkCmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    vkCmdSetCullMode(cmdBuffer, VK_CULL_MODE_NONE);
    vkCmdSetFrontFace(cmdBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

void GraphicPipeline::setFragmentInfo(VkCommandBuffer cmdBuffer) {
    const VkBool32 colorBlendEnables = VK_FALSE;
    const VkColorComponentFlags colorBlendComponentFlags = 0xf;
    const VkColorBlendEquationEXT colorBlendEquation{};

    vkCmdSetColorWriteMaskEXT(cmdBuffer, 0, 1, &colorBlendComponentFlags);
}

void GraphicPipeline::setPipelineStage(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    pipelineStageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    if (!pipelineCreateInfo.geometryShaderFile.empty()) pipelineStageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;

    if (!pipelineCreateInfo.tesselationControlShaderFile.empty()) pipelineStageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    if (!pipelineCreateInfo.tesselationEvaluationShaderFile.empty()) pipelineStageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
}

Shader GraphicPipeline::createFragmentShader(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    Shader fragmentShader(device, pipelineCreateInfo.fragmentShaderFile, pipelineStageFlags, 0);
    // HotReload::addShaderToWatch(pipelineCreateInfo.fragmentShaderFile, this);
    for (auto& descriptorSetlayout : fragmentShader.getDescriptorsSetLayout()) {
        pipelineDescriptorsSetsLayout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return fragmentShader;
}

Shader GraphicPipeline::createVertexShader(GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader vertexShader(device, pipelineCreateInfo.vexterShaderFile, pipelineStageFlags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.vexterShaderFile, this);
    for (auto& descriptorSetlayout : vertexShader.getDescriptorsSetLayout()) {
        pipelineDescriptorsSetsLayout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return vertexShader;
}

Shader GraphicPipeline::createTesselationControlShader(GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader tesselationControlShader(device, pipelineCreateInfo.tesselationControlShaderFile, pipelineStageFlags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.tesselationControlShaderFile, this);
    for (auto& descriptorSetlayout : tesselationControlShader.getDescriptorsSetLayout()) {
        pipelineDescriptorsSetsLayout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return tesselationControlShader;
}

Shader GraphicPipeline::createTesselationEvaluationShader(
    GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader tesselationEvaluationShader(device, pipelineCreateInfo.tesselationEvaluationShaderFile, pipelineStageFlags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.tesselationEvaluationShaderFile, this);
    for (auto& descriptorSetlayout : tesselationEvaluationShader.getDescriptorsSetLayout()) {
        pipelineDescriptorsSetsLayout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return tesselationEvaluationShader;
}

Shader GraphicPipeline::createGeometryShader(GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader geometryShader(device, pipelineCreateInfo.geometryShaderFile, pipelineStageFlags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.geometryShaderFile, this);
    for (auto& descriptorSetlayout : geometryShader.getDescriptorsSetLayout()) {
        pipelineDescriptorsSetsLayout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return geometryShader;
}

void GraphicPipeline::createShaders(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    assert(
        (!pipelineCreateInfo.fragmentShaderFile.empty() && !pipelineCreateInfo.vexterShaderFile.empty()) &&
        "Un vertex et un fragment shader sont requi pour faire une pipeline");

    std::vector<Shader*> buildsShaderVector;
    VkShaderStageFlagBits nextStageFlag;
    if (!pipelineCreateInfo.tesselationEvaluationShaderFile.empty()) {
        nextStageFlag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (!pipelineCreateInfo.geometryShaderFile.empty()) {
        nextStageFlag = VK_SHADER_STAGE_GEOMETRY_BIT;
    } else {
        nextStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    shadersMap[VK_SHADER_STAGE_VERTEX_BIT] = createVertexShader(pipelineCreateInfo, nextStageFlag);
    buildsShaderVector.push_back(&shadersMap[VK_SHADER_STAGE_VERTEX_BIT]);

    if (!pipelineCreateInfo.tesselationEvaluationShaderFile.empty()) {
        nextStageFlag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        shadersMap[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = createTesselationEvaluationShader(pipelineCreateInfo, nextStageFlag);
        buildsShaderVector.push_back(&shadersMap[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT]);

        if (!pipelineCreateInfo.geometryShaderFile.empty()) {
            nextStageFlag = VK_SHADER_STAGE_GEOMETRY_BIT;
        } else {
            nextStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        shadersMap[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = createTesselationControlShader(pipelineCreateInfo, nextStageFlag);
        buildsShaderVector.push_back(&shadersMap[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT]);
    }

    if (!pipelineCreateInfo.geometryShaderFile.empty()) {
        nextStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
        shadersMap[VK_SHADER_STAGE_GEOMETRY_BIT] = createGeometryShader(pipelineCreateInfo, nextStageFlag);
        buildsShaderVector.push_back(&shadersMap[VK_SHADER_STAGE_GEOMETRY_BIT]);
    }
    shadersMap[VK_SHADER_STAGE_FRAGMENT_BIT] = createFragmentShader(pipelineCreateInfo);
    buildsShaderVector.push_back(&shadersMap[VK_SHADER_STAGE_FRAGMENT_BIT]);

    pushConstantInfo = make<VkPushConstantRange>();
    // parcour the shaders to get the push constant
    for (auto& shader : shadersMap) {
        auto shaderPushConstantInfo = shader.second.getPushConstants();
        pushConstantInfo.stageFlags |= shader.first;
        if (shaderPushConstantInfo.size > 0) {
            if (shaderPushConstantInfo.size > pushConstantInfo.size) {
                pushConstantInfo.size = shaderPushConstantInfo.size;
            }
        }
    }

    for (auto& shader : shadersMap) {
        shader.second.setPushConstant(pushConstantInfo);
        shader.second.createShaderInfo();
    }

    // Build shader together
    Shader::buildLinkedShaders(device, buildsShaderVector);
}

void GraphicPipeline::reloadShader(VkShaderStageFlagBits shaderStageToReload) {
    // shadersMap[shaderStageToReload]->reloadShaderData();
    // std::vector<Shader*> buildsShaderVector;
    // for (auto& shader : shadersMap) {
    //     vkDestroyShaderEXT(device->device(), shader.second->getShaderHandler(), nullptr);
    //     buildsShaderVector.push_back(shader.second);
    // }
    // Shader::buildLinkedShaders(device, buildsShaderVector);
}

void GraphicPipeline::createPipelineLayout(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;

    for (auto& descriptorSetLayout : pipelineDescriptorsSetsLayout) {
        pipelineDescriptorsSetsLayoutList[descriptorSetLayout.first[0]] = descriptorSetLayout.second;
    }

    for (auto& descriptorSetLayout : pipelineDescriptorsSetsLayoutList) {
        descriptorSetLayoutVector.push_back(*descriptorSetLayout.second);
    }

    auto pipelineLayoutInfo = make<VkPipelineLayoutCreateInfo>();
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutVector.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayoutVector.data();
    if (pushConstantInfo.size > 0) {
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantInfo;
    }

    if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void GraphicPipeline::createVertexShaderInfo() {
    vertexInputBinding = make<VkVertexInputBindingDescription2EXT>();
    vertexInputBinding.binding = 0;
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBinding.stride = sizeof(Vertex);
    vertexInputBinding.divisor = 1;

    vertexAttributes = {
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 3, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, material_id)}};
}

}  // namespace TTe