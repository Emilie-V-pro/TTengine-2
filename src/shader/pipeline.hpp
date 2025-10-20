#pragma once

#include <cstdint>
#include <memory>

#include "../descriptor/descriptorSetLayout.hpp"
#include "../device.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "volk.h"

namespace TTe {

class Pipeline {
   public:
    virtual void bindPipeline(const CommandBuffer &cmdBuffer) = 0;
    // virtual void reloadShader(VkShaderStageFlagBits shaderStageToReload) = 0;
    std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(uint32_t id) { return m_pipeline_descriptors_sets_layout_list[id]; }
    VkPipelineLayout getPipelineLayout() { return m_vk_pipeline_layout; };
    VkShaderStageFlags getPushConstantStage(){return m_push_constant_info.stageFlags;}

   private:
   std::map<uint32_t, std::shared_ptr<DescriptorSetLayout>> m_pipeline_descriptors_sets_layout_list;
   
   
   protected:
   VkShaderStageFlags m_pipeline_stage_flags = 0;
   
    VkPushConstantRange m_push_constant_info = {};
    VkPipelineLayout m_vk_pipeline_layout = VK_NULL_HANDLE;
    Device *m_device = nullptr;
};

}  // namespace TTe