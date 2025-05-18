#pragma once

#include <cstdint>
#include <memory>

#include "../descriptor/descriptorSetLayout.hpp"
#include "../device.hpp"
#include "../utils.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "volk.h"

namespace TTe {

class Pipeline {
   public:
    virtual void bindPipeline(const CommandBuffer &cmdBuffer) = 0;
    // virtual void reloadShader(VkShaderStageFlagBits shaderStageToReload) = 0;
    std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(uint32_t id) { return pipelineDescriptorsSetsLayoutList[id]; }
    VkPipelineLayout getPipelineLayout() { return vk_pipelineLayout; };
    VkShaderStageFlags getPushConstantStage(){return pushConstantInfo.stageFlags;}

   private:
    VkShaderStageFlags pipelineStageFlags = 0;
    std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>> pipelineDescriptorsSetsLayoutList;
    

   protected:
   
    VkPushConstantRange pushConstantInfo;
    VkPipelineLayout vk_pipelineLayout = VK_NULL_HANDLE;
    Device *device = nullptr;
};

}  // namespace TTe