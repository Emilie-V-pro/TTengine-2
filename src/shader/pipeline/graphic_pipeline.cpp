
#include "graphic_pipeline.hpp"
#include <vulkan/vulkan_core.h>


#include <cstddef>

#include "commandBuffer/command_buffer.hpp"
#include "struct.hpp"
#include "structs_vk.hpp"

namespace TTe {

GraphicPipeline::GraphicPipeline(Device* p_device, GraphicPipelineCreateInfo& p_pipeline_create_info) : m_device(p_device) {
    setPipelineStage(p_pipeline_create_info);
    createShaders(p_pipeline_create_info);
    createPipelineLayout();
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

void GraphicPipeline::bindPipeline(const CommandBuffer& p_cmd_buffer) {
    std::vector<VkShaderEXT> shaders;
    std::vector<VkShaderStageFlagBits> shader_flags;

    for (auto& shader : m_shaders_map) {
        shaders.push_back(shader.second);
        shader_flags.push_back(shader.first);
    }
    vkCmdBindShadersEXT(p_cmd_buffer, shaders.size(), shader_flags.data(), shaders.data());

    setVextexInfo(p_cmd_buffer);
    setRasterizerInfo(p_cmd_buffer);
    // setFragmentInfo(cmdBuffer);
}

void GraphicPipeline::setVextexInfo(VkCommandBuffer p_cmd_buffer) {
    vkCmdSetVertexInputEXT(p_cmd_buffer, 1, &m_vertex_input_binding, m_vertex_attributes.size(), m_vertex_attributes.data());
    vkCmdSetPrimitiveTopology(p_cmd_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdSetPrimitiveRestartEnable(p_cmd_buffer, VK_FALSE);
}

void GraphicPipeline::setRasterizerInfo(VkCommandBuffer p_cmd_buffer) {
    vkCmdSetRasterizationSamplesEXT(p_cmd_buffer, VK_SAMPLE_COUNT_1_BIT);
    uint32_t sampleMask = 0xFF;
    vkCmdSetSampleMaskEXT(p_cmd_buffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vkCmdSetAlphaToCoverageEnableEXT(p_cmd_buffer, VK_FALSE);
    vkCmdSetPolygonModeEXT(p_cmd_buffer, VK_POLYGON_MODE_FILL);
    
    vkCmdSetCullMode(p_cmd_buffer, VK_CULL_MODE_BACK_BIT);
    vkCmdSetFrontFace(p_cmd_buffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

void GraphicPipeline::setFragmentInfo(VkCommandBuffer p_cmd_buffer) {
    // const VkBool32 colorBlendEnables = VK_FALSE;
    const VkColorComponentFlags color_blend_component_flags = 0xf;
    // const VkColorBlendEquationEXT colorBlendEquation{};

    vkCmdSetColorWriteMaskEXT(p_cmd_buffer, 0, 1, &color_blend_component_flags);
}

void GraphicPipeline::setPipelineStage(GraphicPipelineCreateInfo& p_pipeline_create_info) {
    m_pipeline_stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    if (!p_pipeline_create_info.geometry_shader_file.empty()) m_pipeline_stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;

    if (!p_pipeline_create_info.tesselation_control_shader_file.empty()) m_pipeline_stage_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    if (!p_pipeline_create_info.tesselation_control_shader_file.empty()) m_pipeline_stage_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
}

Shader GraphicPipeline::createFragmentShader(GraphicPipelineCreateInfo& p_pipeline_create_info) {
    Shader fragment_shader(m_device, p_pipeline_create_info.fragment_shader_file, m_pipeline_stage_flags, 0);
    // HotReload::addShaderToWatch(p_pipeline_create_info.fragment_shader_file, this);
    for (auto& descriptor_setlayout : fragment_shader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptor_setlayout->getId()] = descriptor_setlayout;
    }
    return fragment_shader;
}

Shader GraphicPipeline::createVertexShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag) {
    Shader vertex_shader(m_device, p_pipeline_create_info.vexter_shader_file, m_pipeline_stage_flags, p_next_stage_flag);
    // HotReload::addShaderToWatch(p_pipeline_create_info.vexter_shader_file, this);
    for (auto& descriptor_setlayout : vertex_shader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptor_setlayout->getId()] = descriptor_setlayout;
    }
    return vertex_shader;
}

Shader GraphicPipeline::createTesselationControlShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag) {
    Shader tesselation_control_shader(m_device, p_pipeline_create_info.tesselation_control_shader_file, m_pipeline_stage_flags, p_next_stage_flag);
    // HotReload::addShaderToWatch(p_pipeline_create_info.tesselation_control_shader_file, this);
    for (auto& descriptor_setlayout : tesselation_control_shader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptor_setlayout->getId()] = descriptor_setlayout;
    }
    return tesselation_control_shader;
}

Shader GraphicPipeline::createTesselationEvaluationShader(
    GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag) {
    Shader tesselation_evaluation_shader(m_device, p_pipeline_create_info.tesselation_evaluation_shader_file, m_pipeline_stage_flags, p_next_stage_flag);
    // HotReload::addShaderToWatch(p_pipeline_create_info.tesselation_evaluation_shader_file, this);
    for (auto& descriptor_setlayout : tesselation_evaluation_shader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptor_setlayout->getId()] = descriptor_setlayout;
    }
    return tesselation_evaluation_shader;
}

Shader GraphicPipeline::createGeometryShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag) {
    Shader geometry_shader(m_device, p_pipeline_create_info.geometry_shader_file, m_pipeline_stage_flags, p_next_stage_flag);
    // HotReload::addShaderToWatch(p_pipeline_create_info.geometry_shader_file, this);
    for (auto& descriptor_setlayout : geometry_shader.getDescriptorsSetLayout()) {
        m_pipeline_descriptors_sets_layout[descriptor_setlayout->getId()] = descriptor_setlayout;
    }
    return geometry_shader;
}

void GraphicPipeline::createShaders(GraphicPipelineCreateInfo& p_pipeline_create_info) {
    assert(
        (!p_pipeline_create_info.fragment_shader_file.empty() && !p_pipeline_create_info.vexter_shader_file.empty()) &&
        "Un vertex et un fragment shader sont requi pour faire une pipeline");

    std::vector<Shader*> builds_shader_vector;
    VkShaderStageFlagBits next_stage_flag;
    if (!p_pipeline_create_info.tesselation_evaluation_shader_file.empty()) {
        next_stage_flag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (!p_pipeline_create_info.geometry_shader_file.empty()) {
        next_stage_flag = VK_SHADER_STAGE_GEOMETRY_BIT;
    } else {
        next_stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    m_shaders_map[VK_SHADER_STAGE_VERTEX_BIT] = createVertexShader(p_pipeline_create_info, next_stage_flag);
    builds_shader_vector.push_back(&m_shaders_map[VK_SHADER_STAGE_VERTEX_BIT]);

    if (!p_pipeline_create_info.tesselation_evaluation_shader_file.empty()) {
        next_stage_flag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        m_shaders_map[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = createTesselationEvaluationShader(p_pipeline_create_info, next_stage_flag);
        builds_shader_vector.push_back(&m_shaders_map[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT]);

        if (!p_pipeline_create_info.geometry_shader_file.empty()) {
            next_stage_flag = VK_SHADER_STAGE_GEOMETRY_BIT;
        } else {
            next_stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        m_shaders_map[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = createTesselationControlShader(p_pipeline_create_info, next_stage_flag);
        builds_shader_vector.push_back(&m_shaders_map[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT]);
    }

    if (!p_pipeline_create_info.geometry_shader_file.empty()) {
        next_stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
        m_shaders_map[VK_SHADER_STAGE_GEOMETRY_BIT] = createGeometryShader(p_pipeline_create_info, next_stage_flag);
        builds_shader_vector.push_back(&m_shaders_map[VK_SHADER_STAGE_GEOMETRY_BIT]);
    }
    m_shaders_map[VK_SHADER_STAGE_FRAGMENT_BIT] = createFragmentShader(p_pipeline_create_info);
    builds_shader_vector.push_back(&m_shaders_map[VK_SHADER_STAGE_FRAGMENT_BIT]);

    m_push_constant_info = make<VkPushConstantRange>();
    // parcour the shaders to get the push constant
    for (auto& shader : m_shaders_map) {
        auto shader_push_constant_info = shader.second.getPushConstants();
        m_push_constant_info.stageFlags |= shader.first;
        if (shader_push_constant_info.size > 0) {
            if (shader_push_constant_info.size > m_push_constant_info.size) {
                m_push_constant_info.size = shader_push_constant_info.size;
            }
        }
    }

    for (auto& shader : m_shaders_map) {
        shader.second.setPushConstant(m_push_constant_info);
        shader.second.createShaderInfo();
    }

    // Build shader together
    Shader::buildLinkedShaders(m_device, builds_shader_vector);
}

void GraphicPipeline::reloadShader(VkShaderStageFlagBits p_shader_stage_to_reload) {
    // m_shaders_map[shaderStageToReload]->reloadShaderData();
    // std::vector<Shader*> buildsShaderVector;
    // for (auto& shader : m_shaders_map) {
    //     vkDestroyShaderEXT(m_device->m_device(), shader.second->getShaderHandler(), nullptr);
    //     buildsShaderVector.push_back(shader.second);
    // }
    // Shader::buildLinkedShaders(m_device, buildsShaderVector);
}

void GraphicPipeline::createPipelineLayout() {
    std::vector<VkDescriptorSetLayout> descriptor_set_layout_vector;

    for (auto& descriptor_set_layout : m_pipeline_descriptors_sets_layout) {
        m_pipeline_descriptors_sets_layout_list[descriptor_set_layout.first[0]] = descriptor_set_layout.second;
    }

    for (auto& descriptor_set_layout : m_pipeline_descriptors_sets_layout_list) {
        descriptor_set_layout_vector.push_back(*descriptor_set_layout.second);
    }

    auto pipeline_layout_info = make<VkPipelineLayoutCreateInfo>();
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layout_vector.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layout_vector.data();
    if (m_push_constant_info.size > 0) {
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &m_push_constant_info;
    }

    if (vkCreatePipelineLayout(*m_device, &pipeline_layout_info, nullptr, &m_vk_pipeline_layout) != VK_SUCCESS) {
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