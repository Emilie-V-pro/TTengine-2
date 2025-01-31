#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../shader.hpp"
#include "../../descriptor//descriptorSetLayout.hpp"
#include "../../device.hpp"
#include "../pipeline.hpp"
#include "volk.h"

namespace TTe {

class ComputePipeline : Pipeline {
   public:
    ComputePipeline(Device* device, std::string computeShaderName);
    
    ~ComputePipeline();

    void bindPipeline(const CommandBuffer& cmdBuffer) ;
    // void reloadShader(VkShaderStageFlagBits shaderStageToReload);

    VkPipelineLayout getPipelineLayout() { return pipelineLayout; };
    std::vector<std::shared_ptr<DescriptorSetLayout>>& getDescriptorsSetLayout(){return  computeShader.getDescriptorsSetLayout();}

   private:
    void createShaders(std::string& computeShaderName);
    void createPipelineLayout();

    Shader computeShader;


    VkPipelineLayout pipelineLayout;

    Device* device;
};

}  // namespace vk_stage