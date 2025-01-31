
#include "compute_pipeline.hpp"

#include "hot_reload.hpp"
#include "structs_vk.hpp"

namespace vk_stage {

ComputePipeline::ComputePipeline(Device* device, std::string computeShaderName) : device(device) { 
    HotReload::addShaderToWatch(computeShaderName, this);
    createShaders(computeShaderName); 
    createPipelineLayout(); 
    }

void ComputePipeline::destroy() {
    computeShader->destroy();
    delete computeShader;
    vkDestroyPipelineLayout(device->device(), pipelineLayout, nullptr);
}

void ComputePipeline::bindPipeline(VkCommandBuffer cmdBuffer) {
    VkShaderEXT sh = computeShader->getShaderHandler();
    VkShaderStageFlagBits sf = VK_SHADER_STAGE_COMPUTE_BIT;
    vkCmdBindShadersEXT(cmdBuffer, 1, &sf, &sh);
}

void ComputePipeline::reloadShader(VkShaderStageFlagBits shaderStageToReload) {
    computeShader->reloadShaderData();
    computeShader->buildShader();
}

void ComputePipeline::createShaders(std::string& computeShaderName) {
    computeShader = new Shader(device, computeShaderName, VK_SHADER_STAGE_COMPUTE_BIT);
    computeShader->buildShader();
}

void ComputePipeline::createPipelineLayout() {
    auto pipelineLayoutInfo = make<VkPipelineLayoutCreateInfo>();
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
    for (auto& descriptorSetLayout : computeShader->getDescriptorsSetLayout()) {
        descriptorSetLayoutVector.push_back(descriptorSetLayout->getDescriptorSetLayout());
    }
    VkPushConstantRange pc = computeShader->getPushConstants();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pc;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutVector.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayoutVector.data();

    if (vkCreatePipelineLayout(device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

}  // namespace vk_stage