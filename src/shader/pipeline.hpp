#pragma once


#include "../descriptor/descriptorSetLayout.hpp"

#include <cstdint>
#include <memory>

#include "../utils.hpp"
#include "../device.hpp"
#include "commandBuffer/command_buffer.hpp"

#include "volk.h"

namespace TTe {


class Pipeline {
   public:
    virtual void bindPipeline(const CommandBuffer &cmdBuffer) = 0;
    //virtual void reloadShader(VkShaderStageFlagBits shaderStageToReload) = 0;
    std::shared_ptr<DescriptorSetLayout> getDescriptorSetLayout(uint32_t id){return pipelineDescriptorsSetsLayoutList[id];}
    VkPipelineLayout getPipelineLayout(){return vk_pipelineLayout;};
    
   private:

    VkShaderStageFlags pipelineStageFlags = 0;
    std::map<uint32_t ,std::shared_ptr<DescriptorSetLayout>> pipelineDescriptorsSetsLayoutList;
    VkPipelineLayout vk_pipelineLayout = VK_NULL_HANDLE;
    Device *device = nullptr;
};

}  // namespace vk_stage