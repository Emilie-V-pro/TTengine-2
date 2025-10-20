
#include "graphic_pipeline.hpp"
#include <vulkan/vulkan_core.h>


#include <cstddef>

#include "commandBuffer/command_buffer.hpp"
#include "struct.hpp"
#include "structs_vk.hpp"

namespace TTe {

GraphicPipeline::GraphicPipeline(Device* device, GraphicPipelineCreateInfo& pipelineCreateInfo) : m_device(device) {
    setPipelineStage(pipelineCreateInfo);
    createShaders(pipelineCreateInfo);
    createPipelineLayout(pipelineCreateInfo);
    createVertexShaderInfo();
}

GraphicPipeline::~GraphicPipeline() {
    // m_shaders_map.clear();
    if(m_vk_pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(*m_device, m_vk_pipeline_layout, nullptr);
}

GraphicPipeline::GraphicPipeline(GraphicPipeline&& other) {
    m_vertex_input_binding = other.m_vertex_input_binding;
    m_vertex_attributes = std::move(other.m_vertex_attributes);
    
    m_shaders_map = std::move(other.m_shaders_map);

    m_pipeline_stage_flags = other.m_pipeline_stage_flags;
    m_pipeline_descriptors_sets_layout = std::move(other.m_pipeline_descriptors_sets_layout);
    m_pipeline_descriptors_sets_layout_list = std::move(other.m_pipeline_descriptors_sets_layout_list);
    m_vk_pipeline_layout = other.m_vk_pipeline_layout;
    m_push_constant_info = other.m_push_constant_info;

    m_device = other.m_device;
    other.m_vk_pipeline_layout = VK_NULL_HANDLE;
}

GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) {
    if (this != &other) {
        m_vertex_input_binding = other.m_vertex_input_binding;
        m_vertex_attributes = std::move(other.m_vertex_attributes);
        
        m_shaders_map = std::move(other.m_shaders_map);

        m_pipeline_stage_flags = other.m_pipeline_stage_flags;
        m_pipeline_descriptors_sets_layout = std::move(other.m_pipeline_descriptors_sets_layout);
        m_pipeline_descriptors_sets_layout_list = std::move(other.m_pipeline_descriptors_sets_layout_list);
        m_vk_pipeline_layout = other.m_vk_pipeline_layout;
        m_push_constant_info = other.m_push_constant_info;

        m_device = other.m_device;
        other.m_vk_pipeline_layout = VK_NULL_HANDLE;
    }
    return *this;
}

void GraphicPipeline::bindPipeline(const CommandBuffer& cmdBuffer) {
    std::vector<VkShaderEXT> shaders;
    std::vector<VkShaderStageFlagBits> shaderFlags;

    for (auto& shader : m_shaders_map) {
        shaders.push_back(shader.second);
        shaderFlags.push_back(shader.first);
    }
    vkCmdBindShadersEXT(cmdBuffer, shaders.size(), shaderFlags.data(), shaders.data());

    setVextexInfo(cmdBuffer);
    setRasterizerInfo(cmdBuffer);
    // setFragmentInfo(cmdBuffer);
}

void GraphicPipeline::setVextexInfo(VkCommandBuffer cmdBuffer) {
    vkCmdSetVertexInputEXT(cmdBuffer, 1, &m_vertex_input_binding, m_vertex_attributes.size(), m_vertex_attributes.data());
    vkCmdSetPrimitiveTopology(cmdBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdSetPrimitiveRestartEnable(cmdBuffer, VK_FALSE);
}

void GraphicPipeline::setRasterizerInfo(VkCommandBuffer cmdBuffer) {
    vkCmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    uint32_t sampleMask = 0xFF;
    vkCmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vkCmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    vkCmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    
    vkCmdSetCullMode(cmdBuffer, VK_CULL_MODE_BACK_BIT);
    vkCmdSetFrontFace(cmdBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

void GraphicPipeline::setFragmentInfo(VkCommandBuffer cmdBuffer) {
    const VkBool32 colorBlendEnables = VK_FALSE;
    const VkColorComponentFlags colorBlendComponentFlags = 0xf;
    const VkColorBlendEquationEXT colorBlendEquation{};

    vkCmdSetColorWriteMaskEXT(cmdBuffer, 0, 1, &colorBlendComponentFlags);
}

void GraphicPipeline::setPipelineStage(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    m_pipeline_stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    if (!pipelineCreateInfo.geometry_shader_file.empty()) m_pipeline_stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;

    if (!pipelineCreateInfo.tesselation_control_shader_file.empty()) m_pipeline_stage_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    if (!pipelineCreateInfo.tesselation_control_shader_file.empty()) m_pipeline_stage_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
}

Shader GraphicPipeline::createFragmentShader(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    Shader fragmentShader(m_device, pipelineCreateInfo.fragment_shader_file, m_pipeline_stage_flags, 0);
    // HotReload::addShaderToWatch(pipelineCreateInfo.fragment_shader_file, this);
    for (auto& descriptorSetlayout : fragmentShader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return fragmentShader;
}

Shader GraphicPipeline::createVertexShader(GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader vertexShader(m_device, pipelineCreateInfo.vexter_shader_file, m_pipeline_stage_flags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.vexter_shader_file, this);
    for (auto& descriptorSetlayout : vertexShader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return vertexShader;
}

Shader GraphicPipeline::createTesselationControlShader(GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader tesselationControlShader(m_device, pipelineCreateInfo.tesselation_control_shader_file, m_pipeline_stage_flags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.tesselation_control_shader_file, this);
    for (auto& descriptorSetlayout : tesselationControlShader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return tesselationControlShader;
}

Shader GraphicPipeline::createTesselationEvaluationShader(
    GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader tesselationEvaluationShader(m_device, pipelineCreateInfo.tesselation_evaluation_shader_file, m_pipeline_stage_flags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.tesselation_evaluation_shader_file, this);
    for (auto& descriptorSetlayout : tesselationEvaluationShader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return tesselationEvaluationShader;
}

Shader GraphicPipeline::createGeometryShader(GraphicPipelineCreateInfo& pipelineCreateInfo, VkShaderStageFlagBits nextStageFlag) {
    Shader geometryShader(m_device, pipelineCreateInfo.geometry_shader_file, m_pipeline_stage_flags, nextStageFlag);
    // HotReload::addShaderToWatch(pipelineCreateInfo.geometry_shader_file, this);
    for (auto& descriptorSetlayout : geometryShader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptorSetlayout->getId()] = descriptorSetlayout;
    }
    return geometryShader;
}

void GraphicPipeline::createShaders(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    assert(
        (!pipelineCreateInfo.fragment_shader_file.empty() && !pipelineCreateInfo.vexter_shader_file.empty()) &&
        "Un vertex et un fragment shader sont requi pour faire une pipeline");

    std::vector<Shader*> buildsShaderVector;
    VkShaderStageFlagBits nextStageFlag;
    if (!pipelineCreateInfo.tesselation_evaluation_shader_file.empty()) {
        nextStageFlag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (!pipelineCreateInfo.geometry_shader_file.empty()) {
        nextStageFlag = VK_SHADER_STAGE_GEOMETRY_BIT;
    } else {
        nextStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    m_shaders_map[VK_SHADER_STAGE_VERTEX_BIT] = createVertexShader(pipelineCreateInfo, nextStageFlag);
    buildsShaderVector.push_back(&m_shaders_map[VK_SHADER_STAGE_VERTEX_BIT]);

    if (!pipelineCreateInfo.tesselation_evaluation_shader_file.empty()) {
        nextStageFlag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        m_shaders_map[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = createTesselationEvaluationShader(pipelineCreateInfo, nextStageFlag);
        buildsShaderVector.push_back(&m_shaders_map[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT]);

        if (!pipelineCreateInfo.geometry_shader_file.empty()) {
            nextStageFlag = VK_SHADER_STAGE_GEOMETRY_BIT;
        } else {
            nextStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        m_shaders_map[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = createTesselationControlShader(pipelineCreateInfo, nextStageFlag);
        buildsShaderVector.push_back(&m_shaders_map[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT]);
    }

    if (!pipelineCreateInfo.geometry_shader_file.empty()) {
        nextStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
        m_shaders_map[VK_SHADER_STAGE_GEOMETRY_BIT] = createGeometryShader(pipelineCreateInfo, nextStageFlag);
        buildsShaderVector.push_back(&m_shaders_map[VK_SHADER_STAGE_GEOMETRY_BIT]);
    }
    m_shaders_map[VK_SHADER_STAGE_FRAGMENT_BIT] = createFragmentShader(pipelineCreateInfo);
    buildsShaderVector.push_back(&m_shaders_map[VK_SHADER_STAGE_FRAGMENT_BIT]);

    m_push_constant_info = make<VkPushConstantRange>();
    // parcour the shaders to get the push constant
    for (auto& shader : m_shaders_map) {
        auto shaderPushConstantInfo = shader.second.getPushConstants();
        m_push_constant_info.stageFlags |= shader.first;
        if (shaderPushConstantInfo.size > 0) {
            if (shaderPushConstantInfo.size > m_push_constant_info.size) {
                m_push_constant_info.size = shaderPushConstantInfo.size;
            }
        }
    }

    for (auto& shader : m_shaders_map) {
        shader.second.setPushConstant(m_push_constant_info);
        shader.second.createShaderInfo();
    }

    // Build shader together
    Shader::buildLinkedShaders(m_device, buildsShaderVector);
}

void GraphicPipeline::reloadShader(VkShaderStageFlagBits shaderStageToReload) {
    // m_shaders_map[shaderStageToReload]->reloadShaderData();
    // std::vector<Shader*> buildsShaderVector;
    // for (auto& shader : m_shaders_map) {
    //     vkDestroyShaderEXT(m_device->m_device(), shader.second->getShaderHandler(), nullptr);
    //     buildsShaderVector.push_back(shader.second);
    // }
    // Shader::buildLinkedShaders(m_device, buildsShaderVector);
}

void GraphicPipeline::createPipelineLayout(GraphicPipelineCreateInfo& pipelineCreateInfo) {
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;

    for (auto& descriptorSetLayout : m_pipeline_descriptors_sets_layout) {
        m_pipeline_descriptors_sets_layout_list[descriptorSetLayout.first[0]] = descriptorSetLayout.second;
    }

    for (auto& descriptorSetLayout : m_pipeline_descriptors_sets_layout_list) {
        descriptorSetLayoutVector.push_back(*descriptorSetLayout.second);
    }

    auto pipelineLayoutInfo = make<VkPipelineLayoutCreateInfo>();
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutVector.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayoutVector.data();
    if (m_push_constant_info.size > 0) {
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &m_push_constant_info;
    }

    if (vkCreatePipelineLayout(*m_device, &pipelineLayoutInfo, nullptr, &m_vk_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void GraphicPipeline::createVertexShaderInfo() {
    m_vertex_input_binding = make<VkVertexInputBindingDescription2EXT>();
    m_vertex_input_binding.binding = 0;
    m_vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_vertex_input_binding.stride = sizeof(Vertex);
    m_vertex_input_binding.divisor = 1;

    m_vertex_attributes = {
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)},
        {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 3, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, material_id)}};
}

}  // namespace TTe