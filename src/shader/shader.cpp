
#include "shader.hpp"

#include <glslang/Include/glslang_c_interface.h>

// Required for use of glslang_default_resource
#include <glslang/Include/glslang_c_shader_types.h>
#include <glslang/Public/resource_limits_c.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "md5.h"
#include "spirv.hpp"
#include "spirv_cross.hpp"
#include "structs_vk.hpp"
#include "volk.h"

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace TTe {
void addSamplerBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &separate_samplers,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : separate_samplers) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
        uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

        if (type.array.size() > 0) {
            binding.descriptorCount = type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        binding.stageFlags = descriptorStage;
        setBindings[set][binding.binding] = binding;
    }
}

void addSampledImagesBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &sampled_images,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : sampled_images) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
        uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

        if (type.array.size() > 0) {
            binding.descriptorCount = type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags = descriptorStage;
        setBindings[set][binding.binding] = binding;
    }
}

void addSeparateImagesBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &separate_images,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : separate_images) {
        if (comp.get_type(resource.type_id).image.dim != spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

            if (type.array.size() > 0) {
                binding.descriptorCount = type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            binding.stageFlags = descriptorStage;
            setBindings[set][binding.binding] = binding;
        }
    }
}

void addStorageImagesBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &storage_images,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : storage_images) {
        if (comp.get_type(resource.type_id).image.dim != spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

            if (type.array.size() > 0) {
                binding.descriptorCount = type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            binding.stageFlags = descriptorStage;
            setBindings[set][binding.binding] = binding;
        }
    }
}

void addUniformTexelBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &separate_images,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : separate_images) {
        if (comp.get_type(resource.type_id).image.dim == spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

            if (type.array.size() > 0) {
                binding.descriptorCount = type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            binding.stageFlags = descriptorStage;
            setBindings[set][binding.binding] = binding;
        }
    }
}

void addStorageTexelBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &storage_images,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : storage_images) {
        if (comp.get_type(resource.type_id).image.dim == spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
            uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

            if (type.array.size() > 0) {
                binding.descriptorCount = type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            binding.stageFlags = descriptorStage;
            setBindings[set][binding.binding] = binding;
        }
    }
}

void addUniformBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &uniform_buffers,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : uniform_buffers) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();

        const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
        uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

        if (type.array.size() > 0) {
            binding.descriptorCount = type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.stageFlags = descriptorStage;
        setBindings[set][binding.binding] = binding;
    }
}

void addStorageBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &storage_buffers,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : storage_buffers) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
        uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

        if (type.array.size() > 0) {
            binding.descriptorCount = type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.stageFlags = descriptorStage;
        setBindings[set][binding.binding] = binding;
    }
}

void addAccelerationStructureBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &acceleration_structures,
    spirv_cross::Compiler &comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &setBindings,
    VkShaderStageFlags descriptorStage) {
    for (const spirv_cross::Resource &resource : acceleration_structures) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &type = comp.get_type(resource.type_id);
        uint32_t set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = comp.get_decoration(resource.id, spv::DecorationBinding);

        if (type.array.size() > 0) {
            binding.descriptorCount = type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        binding.stageFlags = descriptorStage;
        setBindings[set][binding.binding] = binding;
    }
}
}  // namespace TTe

namespace TTe {

Shader::Shader() {}

Shader::Shader(Device *device, std::filesystem::path shaderPath, VkShaderStageFlags descriptorStage, VkShaderStageFlags nextShaderStage)
    : shaderPath(shaderPath), nextShaderStage(nextShaderStage), device(device) {
    shaderStage = getShaderStageFlagsBitFromFileName(shaderPath);
    loadShaderGLSLCode();

    std::filesystem::path shaderFile = std::filesystem::path(shaderPath).filename();
    std::filesystem::path shaderFolder = std::filesystem::path(shaderPath).parent_path();

    if (!std::ifstream((std::filesystem::path(ENGINE_DIR) / shaderFolder / "spirv" / shaderFile).concat(".spv")).good() ||
        getSavedHash() != getShaderSourceCodeHash()) {
        compileToSPIRV();
    }

    loadShaderSPVCode();
    createDescriptorSetLayout(descriptorStage);
    createPushConstant(descriptorStage);
    createShaderInfo();

    // shaderCode.clear();
    shaderSourceCode.clear();
}

Shader::~Shader() {
    if (shader != VK_NULL_HANDLE) {
        vkDestroyShaderEXT(*device, shader, nullptr);
    }
    if (shaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(*device, shaderModule, nullptr);
    }
}

Shader::Shader(Shader &&other) {
    shader = other.shader;
    shaderModule = other.shaderModule;
    shaderCreateInfo = other.shaderCreateInfo;
    pushConstants = other.pushConstants;
    shaderPath = other.shaderPath;
    nextShaderStage = other.nextShaderStage;
    shaderStage = other.shaderStage;
    device = other.device;
    shaderCode = std::move(other.shaderCode);
    descriptorsSetLayout = std::move(other.descriptorsSetLayout);
    computeWorkGroupSize = other.computeWorkGroupSize;
    other.shader = VK_NULL_HANDLE;
    other.shaderModule = VK_NULL_HANDLE;
}

Shader &Shader::operator=(Shader &&other) {
    if (this != &other) {
        if (shader != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(*device, shader, nullptr);
        }
        if (shaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(*device, shaderModule, nullptr);
        }
        shader = other.shader;
        shaderModule = other.shaderModule;
        shaderCreateInfo = other.shaderCreateInfo;
        pushConstants = other.pushConstants;
        shaderPath = other.shaderPath;
        nextShaderStage = other.nextShaderStage;
        shaderStage = other.shaderStage;
        device = other.device;
        shaderCode = std::move(other.shaderCode);
        descriptorsSetLayout = std::move(other.descriptorsSetLayout);
        computeWorkGroupSize = other.computeWorkGroupSize;
        other.shader = VK_NULL_HANDLE;
        other.shaderModule = VK_NULL_HANDLE;
    }
    return *this;
}

void Shader::loadShaderSPVCode() {
    std::filesystem::path shaderFile = std::filesystem::path(shaderPath).filename();
    std::filesystem::path shaderFolder = std::filesystem::path(shaderPath).parent_path();

    shaderCode.clear();
    std::filesystem::path shaderPath = std::filesystem::path(ENGINE_DIR) / shaderFolder / "spirv" / shaderFile;
    shaderPath.concat(".spv");
    std::ifstream file{shaderPath, std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error(
            "failed to open file :" / std::filesystem::path(ENGINE_DIR) / shaderFolder / "spirv" / shaderFile / ".spv");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    shaderCode.resize(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char *>(shaderCode.data()), fileSize);
    file.close();
    shaderCreateInfo.codeSize = shaderCode.size() * sizeof(shaderCode[0]);
    shaderCreateInfo.pCode = shaderCode.data();
}

void Shader::loadShaderGLSLCode() {
    std::ifstream file{ENGINE_DIR / shaderPath, std::ios::ate};
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file :" / std::filesystem::path(ENGINE_DIR) / shaderPath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    shaderSourceCode.resize(fileSize);
    file.seekg(0);
    file.read(shaderSourceCode.data(), fileSize);
    file.close();

    // if last is not \0 add it
    if (shaderSourceCode.back() != '\0') {
        shaderSourceCode.push_back('\0');
    }
}

typedef struct SpirVBinary {
    std::vector<uint32_t> words;  // SPIR-V words
    int size;                     // number of words in SPIR-V binary
} SpirVBinary;

// from glslang readme
void Shader::compileToSPIRV() {
    glslang_input_t input{};
    input.language = GLSLANG_SOURCE_GLSL;
    input.stage = getGLSLangStageFromShaderStage(shaderStage);
    input.client = GLSLANG_CLIENT_VULKAN;
    input.client_version = GLSLANG_TARGET_VULKAN_1_3;
    input.target_language = GLSLANG_TARGET_SPV;
    input.target_language_version = GLSLANG_TARGET_SPV_1_6;
    input.code = shaderSourceCode.data();
    input.default_version = 460;
    input.default_profile = GLSLANG_NO_PROFILE;
    input.forward_compatible = 0;
    input.messages = GLSLANG_MSG_DEFAULT_BIT;
    input.resource = glslang_default_resource();

    glslang_shader_t *shader = glslang_shader_create(&input);

    SpirVBinary bin = {
        .words = {},
        .size = 0,
    };
    if (!glslang_shader_preprocess(shader, &input)) {
        printf("GLSL preprocessing failed %s\n", shaderPath.c_str());
        printf("%s\n", glslang_shader_get_info_log(shader));
        printf("%s\n", glslang_shader_get_info_debug_log(shader));
        printf("%s\n", input.code);
        glslang_shader_delete(shader);
        throw std::runtime_error("GLSL preprocessing failed");
    }

    if (!glslang_shader_parse(shader, &input)) {
        printf("GLSL parsing failed %s\n", shaderPath.c_str());
        printf("%s\n", glslang_shader_get_info_log(shader));
        printf("%s\n", glslang_shader_get_info_debug_log(shader));
        printf("%s\n", glslang_shader_get_preprocessed_code(shader));
        glslang_shader_delete(shader);
        throw std::runtime_error("GLSL parsing failed");
    }

    glslang_program_t *program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        printf("GLSL linking failed %s\n", shaderPath.c_str());
        printf("%s\n", glslang_program_get_info_log(program));
        printf("%s\n", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        throw std::runtime_error("GLSL linking failed");
    }

    glslang_program_SPIRV_generate(program, input.stage);

    bin.size = glslang_program_SPIRV_get_size(program);
    bin.words.resize(bin.size);
    glslang_program_SPIRV_get(program, bin.words.data());

    const char *spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages) printf("(%s) %s\b", shaderPath.c_str(), spirv_messages);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    std::filesystem::path shaderFile = std::filesystem::path(shaderPath).filename();
    std::filesystem::path shaderFolder = std::filesystem::path(shaderPath).parent_path();

    // write SPIR-V binary to file
    std::ofstream outFile((std::filesystem::path(ENGINE_DIR) / shaderFolder / "spirv" / shaderFile).concat(".spv"), std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error(
            "failed to open file for writing SPIR-V binary: " / std::filesystem::path(ENGINE_DIR) / shaderFolder / "spirv" / shaderFile /
            ".spv");
    }
    outFile.write(reinterpret_cast<const char *>(bin.words.data()), bin.size * sizeof(uint32_t));
    outFile.close();

    std::cout << "Shader compiled to SPIR-V: " << shaderFile << std::endl;
    saveHash();
}

std::string Shader::getShaderSourceCodeHash() {
    Chocobo1::MD5 md5;

    md5.addData(shaderSourceCode.data(), shaderSourceCode.size());
    return md5.finalize().toString();
}

void Shader::saveHash() {
    std::string hash = getShaderSourceCodeHash();

    std::filesystem::path shaderFile = std::filesystem::path(shaderPath).filename();
    std::filesystem::path shaderFolder = std::filesystem::path(shaderPath).parent_path();

    std::ofstream hashFile((std::filesystem::path(ENGINE_DIR) / shaderFolder / "hash" / shaderFile).concat(".hash"), std::ios::binary);
    if (!hashFile.is_open()) {
        throw std::runtime_error(
            "failed to open file for writing hash: " /
            (std::filesystem::path(ENGINE_DIR) / shaderFolder / "hash" / shaderFile).concat(".hash"));
    }
    hashFile.write(hash.data(), hash.size());
    hashFile.close();
}

std::string Shader::getSavedHash() {
    std::filesystem::path shaderFile = std::filesystem::path(shaderPath).filename();
    std::filesystem::path shaderFolder = std::filesystem::path(shaderPath).parent_path();

    std::ifstream hashFile((std::filesystem::path(ENGINE_DIR) / shaderFolder / "hash" / shaderFile).concat(".hash"), std::ios::binary);
    if (!hashFile.is_open()) {
        throw std::runtime_error(
            "failed to open file for reading hash: " / (std::filesystem::path(ENGINE_DIR) / shaderFolder / "hash" / shaderFile).concat(".hash"));
    }
    std::string hash;
    hashFile.seekg(0, std::ios::end);
    hash.resize(hashFile.tellg());
    hashFile.seekg(0, std::ios::beg);
    hashFile.read(hash.data(), hash.size());
    hashFile.close();
    return hash;
}

void Shader::createDescriptorSetLayout(VkShaderStageFlags descriptorStage) {
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> setBindings;
    spirv_cross::Compiler comp(shaderCode);
    spirv_cross::ShaderResources res = comp.get_shader_resources();

    comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 0);
    addSamplerBinding(res.separate_samplers, comp, setBindings, descriptorStage);
    addSampledImagesBinding(res.sampled_images, comp, setBindings, descriptorStage);
    addSeparateImagesBinding(res.separate_images, comp, setBindings, descriptorStage);
    addStorageImagesBinding(res.storage_images, comp, setBindings, descriptorStage);
    addUniformTexelBufferBinding(res.separate_images, comp, setBindings, descriptorStage);
    addStorageTexelBufferBinding(res.storage_images, comp, setBindings, descriptorStage);
    addUniformBufferBinding(res.uniform_buffers, comp, setBindings, descriptorStage);
    addStorageBufferBinding(res.storage_buffers, comp, setBindings, descriptorStage);
    addAccelerationStructureBinding(res.acceleration_structures, comp, setBindings, descriptorStage);
    if (shaderStage == VK_SHADER_STAGE_COMPUTE_BIT) {
        computeWorkGroupSize.width = comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 0);
        computeWorkGroupSize.height = comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 1);
        computeWorkGroupSize.depth = comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 2);
    }

    for (auto &descriptorSet : setBindings) {
        descriptorsSetLayout.push_back(DescriptorSetLayout::createDescriptorSetLayout(device, descriptorSet.second, descriptorSet.first));
    }
}

void Shader::createPushConstant(VkShaderStageFlags descriptorStage) {
    spirv_cross::Compiler comp(shaderCode);

    pushConstants = make<VkPushConstantRange>();
    pushConstants.offset = 0;
    pushConstants.stageFlags = descriptorStage;
    auto ranges = comp.get_shader_resources().push_constant_buffers;
    for (auto &res : ranges) {
        auto &type = comp.get_type(res.base_type_id);
        uint32_t size = comp.get_declared_struct_size(type);
        pushConstants.size = size;
    }
}

void Shader::createShaderInfo() {
    // listDescriptor = new VkDescriptorSetLayout[descriptorsSetLayout.size()];
    // for (int i = 0; i < descriptorsSetLayout.size(); i++) {
    //     listDescriptor[i] = *descriptorsSetLayout[i];
    // }

    shaderCreateInfo = make<VkShaderCreateInfoEXT>();
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.stage = shaderStage;
    shaderCreateInfo.nextStage = nextShaderStage;
    shaderCreateInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    shaderCreateInfo.codeSize = shaderCode.size() * sizeof(shaderCode[0]);
    shaderCreateInfo.pCode = shaderCode.data();
    shaderCreateInfo.pName = "main";
    shaderCreateInfo.setLayoutCount = descriptorsSetLayout.size();
    // shaderCreateInfo.pSetLayouts = listDescriptor;
    shaderCreateInfo.pushConstantRangeCount = (pushConstants.size > 0) ? 1 : 0;
    shaderCreateInfo.pPushConstantRanges = (pushConstants.size > 0) ? &pushConstants : nullptr;
}

// Creation of shader module for Ray-Tracing
void Shader::createShaderModule() {
    auto moduleCreateInfo = make<VkShaderModuleCreateInfo>();
    moduleCreateInfo.codeSize = shaderCode.size() * sizeof(shaderCode[0]);
    moduleCreateInfo.pCode = shaderCode.data();
    auto res = vkCreateShaderModule(*device, &moduleCreateInfo, NULL, &shaderModule);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }
}

void Shader::buildShader() {
    std::vector<VkDescriptorSetLayout> listDescriptor;
    for (size_t i = 0; i < descriptorsSetLayout.size(); i++) {
        listDescriptor.push_back(*descriptorsSetLayout[i]);
    }
    shaderCreateInfo.pSetLayouts = listDescriptor.data();

    VkResult result = vkCreateShadersEXT(*device, 1, &shaderCreateInfo, nullptr, &shader);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to build shader");
    }
}

void Shader::buildLinkedShaders(Device *device, std::vector<Shader *> &shaders) {
    std::vector<VkShaderCreateInfoEXT> shadersCreateInfos;
    std::vector<std::vector<VkDescriptorSetLayout>> listDescriptor(shaders.size());
    for (size_t i = 0; i < shaders.size(); i++) {
        shadersCreateInfos.push_back(shaders[i]->getShaderCreateInfo());
        shadersCreateInfos[i].flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
        for (size_t j = 0; j < shaders[i]->getDescriptorsSetLayout().size(); j++) {
            listDescriptor[i].push_back(*shaders[i]->getDescriptorsSetLayout()[j]);
        }
        shadersCreateInfos[i].setLayoutCount = listDescriptor[i].size();
        shadersCreateInfos[i].pSetLayouts = listDescriptor[i].data();
    }

    std::vector<VkShaderEXT> shaderHandler(shaders.size());
    VkResult result = vkCreateShadersEXT(*device, shadersCreateInfos.size(), shadersCreateInfos.data(), nullptr, shaderHandler.data());

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader");
    }
    for (size_t i = 0; i < shaderHandler.size(); i++) {
        shaders[i]->setShaderHandler(shaderHandler[i]);
    }
}

constexpr unsigned int str2int(const char *str, int h = 0) { return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h]; }

VkShaderStageFlagBits Shader::getShaderStageFlagsBitFromFileName(std::filesystem::path shaderFile) {
    switch (str2int(shaderFile.extension().c_str())) {
        case (str2int(".vert")):
            return VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case (str2int(".tesc")):
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            break;
        case (str2int(".tese")):
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            break;
        case (str2int(".geom")):
            return VK_SHADER_STAGE_GEOMETRY_BIT;
            break;
        case (str2int(".frag")):
            return VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case (str2int(".comp")):
            return VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        case (str2int(".rgen")):
            return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            break;
        case (str2int(".rint")):
            return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
            break;
        case (str2int(".rahit")):
            return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            break;
        case (str2int(".rchit")):
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            break;
        case (str2int(".rmiss")):
            return VK_SHADER_STAGE_MISS_BIT_KHR;
            break;
        case (str2int(".rcall")):
            return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
            break;
        default:
            throw std::runtime_error("not supported shader type");
    }
}

glslang_stage_t Shader::getGLSLangStageFromShaderStage(VkShaderStageFlagBits shaderStage) const {
    switch (shaderStage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return GLSLANG_STAGE_VERTEX;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return GLSLANG_STAGE_TESSCONTROL;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return GLSLANG_STAGE_TESSCONTROL;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return GLSLANG_STAGE_GEOMETRY;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return GLSLANG_STAGE_FRAGMENT;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return GLSLANG_STAGE_COMPUTE;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            return GLSLANG_STAGE_RAYGEN;
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            return GLSLANG_STAGE_INTERSECT;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            return GLSLANG_STAGE_ANYHIT;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            return GLSLANG_STAGE_CLOSESTHIT;
        case VK_SHADER_STAGE_MISS_BIT_KHR:
            return GLSLANG_STAGE_MISS;
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return GLSLANG_STAGE_CALLABLE;
        default:
            throw std::runtime_error("not supported shader stage");
    }
}

}  // namespace TTe
