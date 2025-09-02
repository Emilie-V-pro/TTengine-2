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
    ComputePipeline(Device* device, std::filesystem::path computeShaderPath);
    // Destructor
    ~ComputePipeline();

    // Remove copy constructor
    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline& operator=(const ComputePipeline&) = delete;

    // Move constructor
    ComputePipeline(ComputePipeline&& other);
    ComputePipeline& operator=(ComputePipeline&& other);


    void bindPipeline(const CommandBuffer& cmdBuffer);
    void dispatch(const CommandBuffer& cmdBuffer, uint32_t nbOfinvocationX = 1, uint32_t nbOfinvocationY = 1, uint32_t nbOfinvocationZ = 1);

    // void reloadShader(VkShaderStageFlagBits shaderStageToReload);

    VkPipelineLayout getPipelineLayout() { return pipelineLayout; };
    std::vector<std::shared_ptr<DescriptorSetLayout>>& getDescriptorsSetLayout() { return computeShader.getDescriptorsSetLayout(); }

   private:
    void createShaders(std::filesystem::path& computeShaderPath);
    void createPipelineLayout();

    Shader computeShader;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    Device* device = nullptr;
};

}  // namespace TTe