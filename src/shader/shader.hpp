#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../device.hpp"
#include "../descriptor/descriptorSetLayout.hpp"
#include "volk.h"
namespace TTe {
class Shader {
   public:
    Shader(Device *device, std::string shaderFile, VkShaderStageFlags descriptorStage, VkShaderStageFlags nextShaderStage = 0);
    ~Shader();

    void buildShader();
    void createShaderModule();

    static void buildLinkedShaders(Device *device, std::vector<Shader *> shaders);

    static VkShaderStageFlagBits getShaderStageFlagsBitFromFileName(std::string shaderFile);

    VkShaderStageFlagBits getShaderStage() { return shaderStage; }
    VkShaderStageFlags getNexShadersStages() { return nextShaderStage; }
    VkShaderCreateInfoEXT getShaderCreateInfo() { return shaderCreateInfo; }
    VkPushConstantRange getPushConstants() { return pushConstants; }
    VkShaderEXT getShaderHandler() { return shader; }
    VkShaderModule getShaderModule() { return shaderModule;}

    std::vector<std::shared_ptr<DescriptorSetLayout>> &getDescriptorsSetLayout() { return descriptorsSetLayout; }



    void setShaderHandler(VkShaderEXT shaderExt) { shader = shaderExt; }


   private:
    void loadShaderCode();
    void createDescriptorSetLayout(VkShaderStageFlags descriptorStage);
    void createPushConstant(VkShaderStageFlags descriptorStage);
    void createShaderInfo();

    std::vector<uint32_t> shaderCode;

    std::vector<std::shared_ptr<DescriptorSetLayout>> descriptorsSetLayout;
    VkPushConstantRange pushConstants;

    std::string shaderFile;
    VkShaderStageFlags nextShaderStage;
    VkShaderStageFlagBits shaderStage;

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    VkShaderCreateInfoEXT shaderCreateInfo;

    VkShaderEXT shader;
    Device *device;
};

}  // namespace vk_stage