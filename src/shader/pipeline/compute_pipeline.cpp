
#include "compute_pipeline.hpp"
#include <cstdint>


#include "structs_vk.hpp"

namespace TTe {

ComputePipeline::ComputePipeline(Device* p_device, std::filesystem::path p_compute_shader_name) : m_device(p_device) {
    createShaders(p_compute_shader_name);
    createPipelineLayout();
}

ComputePipeline::~ComputePipeline() {
    vkDestroyPipelineLayout(*m_device, m_pipeline_layout, nullptr);
}

ComputePipeline::ComputePipeline(ComputePipeline&& other) {
    m_compute_shader = std::move(other.m_compute_shader);
    m_pipeline_layout = other.m_pipeline_layout;
    m_device = other.m_device;
    m_push_constant_info = other.m_push_constant_info;
    other.m_pipeline_layout = VK_NULL_HANDLE;
}

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) {
    if (this != &other) {
        if (m_pipeline_layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(*m_device, m_pipeline_layout, nullptr);
        }
        m_compute_shader = std::move(other.m_compute_shader);
        m_pipeline_layout = other.m_pipeline_layout;
        m_device = other.m_device;
        m_push_constant_info = other.m_push_constant_info;
        other.m_pipeline_layout = VK_NULL_HANDLE;
    }
    return *this;
}

void ComputePipeline::bindPipeline(const CommandBuffer& p_cmd_buffer) {
    VkShaderEXT sh = m_compute_shader;
    VkShaderStageFlagBits sf = VK_SHADER_STAGE_COMPUTE_BIT;
    vkCmdBindShadersEXT(p_cmd_buffer, 1, &sf, &sh);
}

void ComputePipeline::dispatch(const CommandBuffer& p_cmd_buffer, uint32_t p_nb_of_invocation_x, uint32_t p_nb_of_invocation_y, uint32_t p_nb_of_invocation_z) {
    // calculate the number of workgroups
    VkExtent3D work_group_size = m_compute_shader.getComputeWorkGroupSize();
    uint32_t work_group_count_x = p_nb_of_invocation_x / work_group_size.width + ((p_nb_of_invocation_x % work_group_size.width != 0) ? 1 : 0);
    uint32_t work_group_count_y = p_nb_of_invocation_y / work_group_size.height + ((p_nb_of_invocation_y % work_group_size.height != 0) ? 1 : 0);
    uint32_t work_group_count_z = p_nb_of_invocation_z / work_group_size.depth + ((p_nb_of_invocation_z % work_group_size.depth != 0) ? 1 : 0);
    

    vkCmdDispatch(p_cmd_buffer, work_group_count_x, work_group_count_y, work_group_count_z);
}


void ComputePipeline::createShaders(std::filesystem::path& p_compute_shader_path) {
    m_compute_shader = Shader(m_device, p_compute_shader_path, VK_SHADER_STAGE_COMPUTE_BIT);
    m_compute_shader.buildShader();

    m_push_constant_info = m_compute_shader.getPushConstants();
}

void ComputePipeline::createPipelineLayout() {
    auto pipeline_layout_info = make<VkPipelineLayoutCreateInfo>();
    std::vector<VkDescriptorSetLayout> descriptor_set_layout_vector;
    for (auto& descriptor_set_layout : m_compute_shader.getDescriptorsSetLayout()) {
        descriptor_set_layout_vector.push_back(*descriptor_set_layout);
    }
    VkPushConstantRange pc = m_compute_shader.getPushConstants();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &pc;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layout_vector.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layout_vector.data();

    if (vkCreatePipelineLayout(*m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

}  // namespace TTe