#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../descriptor/descriptorSetLayout.hpp"
#include "../device.hpp"
#include "volk.h"
namespace TTe {
class Shader {
   public:
    // Constructor
    Shader();
    Shader(Device *device, std::string shaderFile, VkShaderStageFlags descriptorStage, VkShaderStageFlags nextShaderStage = 0);

    // Destructor
    ~Shader();

    // Remove copy constructor
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    // Move constructor
    Shader(Shader &&other);
    Shader &operator=(Shader &&other);

    void createShaderModule();

    void buildShader();
    static void buildLinkedShaders(Device *device, std::vector<Shader *> shaders);

    // Getters
    operator VkShaderEXT() { return shader; }
    operator VkShaderModule() { return shaderModule; }

    const VkShaderStageFlagBits &getShaderStage() const { return shaderStage; }
    const VkShaderStageFlags &getNexShadersStages() const { return nextShaderStage; }
    const VkShaderCreateInfoEXT &getShaderCreateInfo() const { return shaderCreateInfo; }
    const VkPushConstantRange &getPushConstants() const { return pushConstants; }
    const VkExtent3D &getComputeWorkGroupSize() const { return computeWorkGroupSize; }
    std::vector<std::shared_ptr<DescriptorSetLayout>> &getDescriptorsSetLayout() { return descriptorsSetLayout; }

    static VkShaderStageFlagBits getShaderStageFlagsBitFromFileName(std::string shaderFile);

    void setShaderHandler(VkShaderEXT shaderExt) { shader = shaderExt; }

   private:
    void loadShaderCode();
    void createDescriptorSetLayout(VkShaderStageFlags descriptorStage);
    void createPushConstant(VkShaderStageFlags descriptorStage);
    void createShaderInfo();

    std::vector<uint32_t> shaderCode;

    std::vector<std::shared_ptr<DescriptorSetLayout>> descriptorsSetLayout;

    VkPushConstantRange pushConstants = {};

    std::string shaderFile;
    VkShaderStageFlags nextShaderStage = 0;
    VkShaderStageFlagBits shaderStage;

    VkExtent3D computeWorkGroupSize = {1, 1, 1};


    VkShaderCreateInfoEXT shaderCreateInfo = {};

    VkShaderEXT shader = VK_NULL_HANDLE;
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    Device *device = nullptr;
};

}  // namespace TTe