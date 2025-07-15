
#include "compute_pipeline.hpp"
#include <cstdint>


#include "structs_vk.hpp"

namespace TTe {

ComputePipeline::ComputePipeline(Device* device, std::string computeShaderName) : device(device) {
    createShaders(computeShaderName);
    createPipelineLayout();
}

ComputePipeline::~ComputePipeline() {
    vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
}

ComputePipeline::ComputePipeline(ComputePipeline&& other) {
    computeShader = std::move(other.computeShader);
    pipelineLayout = other.pipelineLayout;
    device = other.device;
    other.pipelineLayout = VK_NULL_HANDLE;
}

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) {
    if (this != &other) {
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
        }
        computeShader = std::move(other.computeShader);
        pipelineLayout = other.pipelineLayout;
        device = other.device;
        other.pipelineLayout = VK_NULL_HANDLE;
    }
    return *this;
}

void ComputePipeline::bindPipeline(const CommandBuffer& cmdBuffer) {
    VkShaderEXT sh = computeShader;
    VkShaderStageFlagBits sf = VK_SHADER_STAGE_COMPUTE_BIT;
    vkCmdBindShadersEXT(cmdBuffer, 1, &sf, &sh);
}

void ComputePipeline::dispatch(const CommandBuffer& cmdBuffer, uint32_t nbOfinvocationX, uint32_t nbOfinvocationY, uint32_t nbOfinvocationZ) {
    // calculate the number of workgroups
    VkExtent3D workGroupSize = computeShader.getComputeWorkGroupSize();
    uint32_t workGroupCountX = nbOfinvocationX / workGroupSize.width + ((nbOfinvocationX % workGroupSize.width != 0) ? 1 : 0);
    uint32_t workGroupCountY = nbOfinvocationY / workGroupSize.height + ((nbOfinvocationY % workGroupSize.height != 0) ? 1 : 0);
    uint32_t workGroupCountZ = nbOfinvocationZ / workGroupSize.depth + ((nbOfinvocationZ % workGroupSize.depth != 0) ? 1 : 0);
    

    vkCmdDispatch(cmdBuffer, workGroupCountX, workGroupCountY, workGroupCountZ);
}


void ComputePipeline::createShaders(std::string& computeShaderName) {
    computeShader = Shader(device, computeShaderName, VK_SHADER_STAGE_COMPUTE_BIT);
    computeShader.buildShader();
}

void ComputePipeline::createPipelineLayout() {
    auto pipelineLayoutInfo = make<VkPipelineLayoutCreateInfo>();
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutVector;
    for (auto& descriptorSetLayout : computeShader.getDescriptorsSetLayout()) {
        descriptorSetLayoutVector.push_back(*descriptorSetLayout);
    }
    VkPushConstantRange pc = computeShader.getPushConstants();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pc;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutVector.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayoutVector.data();

    if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

}  // namespace TTe