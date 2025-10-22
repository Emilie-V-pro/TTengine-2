#pragma once

#include "commandBuffer/command_buffer.hpp"
#include "shader/pipeline.hpp"
#include "shader/shader.hpp"
#include "utils.hpp"

namespace TTe {
struct GraphicPipelineCreateInfo{
    std::filesystem::path vexter_shader_file;
    std::filesystem::path fragment_shader_file;
    std::filesystem::path tesselation_control_shader_file;
    std::filesystem::path tesselation_evaluation_shader_file;
    std::filesystem::path geometry_shader_file;
};

class GraphicPipeline : public Pipeline {

   public:
    GraphicPipeline(){};
    GraphicPipeline(Device *p_device, GraphicPipelineCreateInfo& p_pipeline_create_info);
    ~GraphicPipeline();


    // Move constructor
    GraphicPipeline(GraphicPipeline &&other);
    GraphicPipeline &operator=(GraphicPipeline &&other);


    VkPipelineLayout getPipelineLayout(){return m_vk_pipeline_layout;};
    std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(uint32_t p_id){return m_pipeline_descriptors_sets_layout_list[p_id];}
    

    void bindPipeline(const CommandBuffer &p_cmd_buffer);
    void reloadShader(VkShaderStageFlagBits p_shader_stage_to_reload);
   private:

    void createShaders(GraphicPipelineCreateInfo& p_pipeline_create_info);

    Shader  createFragmentShader(GraphicPipelineCreateInfo& p_pipeline_create_info);
    Shader  createVertexShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag);
    Shader  createTesselationControlShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag);
    Shader  createTesselationEvaluationShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag);
    Shader  createGeometryShader(GraphicPipelineCreateInfo& p_pipeline_create_info, VkShaderStageFlagBits p_next_stage_flag);

    void setPipelineStage(GraphicPipelineCreateInfo& p_pipeline_create_info);
    void createPipelineLayout();
    void createVertexShaderInfo();

    void setVextexInfo(VkCommandBuffer p_cmd_buffer);

    void setRasterizerInfo(VkCommandBuffer p_cmd_buffer);

    void setFragmentInfo(VkCommandBuffer p_cmd_buffer);

    VkVertexInputBindingDescription2EXT m_vertex_input_binding = {};
    std::vector<VkVertexInputAttributeDescription2EXT> m_vertex_attributes;

    std::map<VkShaderStageFlagBits, Shader> m_shaders_map;
    
    std::unordered_map<std::vector<uint32_t>, std::shared_ptr<DescriptorSetLayout>> m_pipeline_descriptors_sets_layout;
    std::map<uint32_t ,std::shared_ptr<DescriptorSetLayout>> m_pipeline_descriptors_sets_layout_list;

    

    Device *m_device = nullptr;
};

}  // namespace vk_stage