#pragma once

#include <memory>
#include <vector>

#include "../../descriptor//descriptorSetLayout.hpp"
#include "../../device.hpp"
#include "../pipeline.hpp"
#include "../shader.hpp"
#include "volk.h"

namespace TTe {

class ComputePipeline : public Pipeline {
   public:
   ComputePipeline() {};
    // Constructor
    ComputePipeline(Device* p_device, std::filesystem::path p_compute_shader_path);
    // Destructor
    ~ComputePipeline();

    // Remove copy constructor
    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline& operator=(const ComputePipeline&) = delete;

    // Move constructor
    ComputePipeline(ComputePipeline&& other);
    ComputePipeline& operator=(ComputePipeline&& other);


    void bindPipeline(const CommandBuffer& p_cmd_buffer);
    void dispatch(const CommandBuffer& p_cmd_buffer, uint32_t p_nb_of_invocation_x = 1, uint32_t p_nb_of_invocation_y = 1, uint32_t p_nb_of_invocation_z = 1);

    // void reloadShader(VkShaderStageFlagBits shaderStageToReload);

    VkPipelineLayout getPipelineLayout() { return m_pipeline_layout; };
    std::vector<std::shared_ptr<DescriptorSetLayout>>& getDescriptorsSetLayout() { return m_compute_shader.getDescriptorsSetLayout(); }

   private:
    void createShaders(std::filesystem::path& p_compute_shader_path);
    void createPipelineLayout();

    Shader m_compute_shader;

    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;

    Device* m_device = nullptr;
};

}  // namespace TTe