#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <glslang/Include/glslang_c_interface.h>

#include "../descriptor/descriptorSetLayout.hpp"
#include "../device.hpp"
#include "volk.h"
namespace TTe {
class Shader {
   public:
    // Constructor
    Shader();
    Shader(Device *p_device, std::filesystem::path p_shader_file, VkShaderStageFlags p_descriptor_stage, VkShaderStageFlags p_next_shader_stage = 0);

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
    static void buildLinkedShaders(Device *p_device, std::vector<Shader *> &p_shaders);

    // Getters
    operator VkShaderEXT() { return m_shader; }
    operator VkShaderModule() { return m_shader_module; }

    const VkShaderStageFlagBits &getShaderStage() const { return m_shader_stage; }
    const VkShaderStageFlags &getNexShadersStages() const { return m_next_shader_stage; }
    const VkShaderCreateInfoEXT &getShaderCreateInfo() const { return m_shader_create_info; }
    const VkPushConstantRange &getPushConstants() const { return m_push_constants; }
    const VkExtent3D &getComputeWorkGroupSize() const { return m_compute_work_group_size; }
    std::vector<std::shared_ptr<DescriptorSetLayout>> &getDescriptorsSetLayout() { return m_descriptors_set_layout; }

    static VkShaderStageFlagBits getShaderStageFlagsBitFromFileName(std::filesystem::path p_shader_file);
    

    void setShaderHandler(VkShaderEXT p_shader_ext) { m_shader = p_shader_ext; }
    void setPushConstant(VkPushConstantRange p_push_constant) { m_push_constants = p_push_constant; }
    void createShaderInfo();
   protected:
    void loadShaderSPVCode();
    void loadShaderGLSLCode();
    void compileToSPIRV();
    
    std::string getShaderSourceCodeHash();
    void saveHash();
    std::string getSavedHash();

    void createDescriptorSetLayout(VkShaderStageFlags p_descriptor_stage);
    void createPushConstant(VkShaderStageFlags p_descriptor_stage);

    glslang_stage_t getGLSLangStageFromShaderStage(VkShaderStageFlagBits p_shader_stage) const;

    std::vector<uint32_t> m_shader_code;
    std::vector<char> m_shader_source_code;

    std::vector<std::shared_ptr<DescriptorSetLayout>> m_descriptors_set_layout;

    VkPushConstantRange m_push_constants = {};

    std::filesystem::path m_shader_path;
    VkShaderStageFlags m_next_shader_stage = 0;
    VkShaderStageFlagBits m_shader_stage;

    VkExtent3D m_compute_work_group_size = {1, 1, 1};


    VkShaderCreateInfoEXT m_shader_create_info = {};

    VkShaderEXT m_shader = VK_NULL_HANDLE;
    VkShaderModule m_shader_module = VK_NULL_HANDLE;
    Device *m_device = nullptr;
};

}  // namespace TTe