
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
    spirv_cross::SmallVector<spirv_cross::Resource> &p_separate_samplers,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_separate_samplers) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
        uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

        if (m_type.array.size() > 0) {
            binding.descriptorCount = m_type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        binding.stageFlags = descriptor_stage;
        p_set_bindings[set][binding.binding] = binding;
    }
}

void addSampledImagesBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_sampled_images,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_sampled_images) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
        uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

        if (m_type.array.size() > 0) {
            binding.descriptorCount = m_type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags = descriptor_stage;
        p_set_bindings[set][binding.binding] = binding;
    }
}

void addSeparateImagesBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_separate_images,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_separate_images) {
        if (p_comp.get_type(resource.type_id).image.dim != spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
            uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

            if (m_type.array.size() > 0) {
                binding.descriptorCount = m_type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            binding.stageFlags = descriptor_stage;
            p_set_bindings[set][binding.binding] = binding;
        }
    }
}

void addStorageImagesBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_storage_images,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_storage_images) {
        if (p_comp.get_type(resource.type_id).image.dim != spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
            uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

            if (m_type.array.size() > 0) {
                binding.descriptorCount = m_type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            binding.stageFlags = descriptor_stage;
            p_set_bindings[set][binding.binding] = binding;
        }
    }
}

void addUniformTexelBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_separate_images,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_separate_images) {
        if (p_comp.get_type(resource.type_id).image.dim == spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
            uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

            if (m_type.array.size() > 0) {
                binding.descriptorCount = m_type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            binding.stageFlags = descriptor_stage;
            p_set_bindings[set][binding.binding] = binding;
        }
    }
}

void addStorageTexelBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_storage_images,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_storage_images) {
        if (p_comp.get_type(resource.type_id).image.dim == spv::DimBuffer) {
            VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
            const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
            uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

            if (m_type.array.size() > 0) {
                binding.descriptorCount = m_type.array[0];
                if (binding.descriptorCount == 1) {
                    binding.descriptorCount = 1000;
                }
            } else {
                binding.descriptorCount = 1;
            }
            binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            binding.stageFlags = descriptor_stage;
            p_set_bindings[set][binding.binding] = binding;
        }
    }
}

void addUniformBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_uniform_buffers,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_uniform_buffers) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();

        const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
        uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

        if (m_type.array.size() > 0) {
            binding.descriptorCount = m_type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.stageFlags = descriptor_stage;
        p_set_bindings[set][binding.binding] = binding;
    }
}

void addStorageBufferBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_storage_buffers,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_storage_buffers) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
        uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

        if (m_type.array.size() > 0) {
            binding.descriptorCount = m_type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.stageFlags = descriptor_stage;
        p_set_bindings[set][binding.binding] = binding;
    }
}

void addAccelerationStructureBinding(
    spirv_cross::SmallVector<spirv_cross::Resource> &p_acceleration_structures,
    spirv_cross::Compiler &p_comp,
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> &p_set_bindings,
    VkShaderStageFlags descriptor_stage) {
    for (const spirv_cross::Resource &resource : p_acceleration_structures) {
        VkDescriptorSetLayoutBinding binding = make<VkDescriptorSetLayoutBinding>();
        const spirv_cross::SPIRType &m_type = p_comp.get_type(resource.type_id);
        uint32_t set = p_comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = p_comp.get_decoration(resource.id, spv::DecorationBinding);

        if (m_type.array.size() > 0) {
            binding.descriptorCount = m_type.array[0];
            if (binding.descriptorCount == 1) {
                binding.descriptorCount = 1000;
            }
        } else {
            binding.descriptorCount = 1;
        }
        binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        binding.stageFlags = descriptor_stage;
        p_set_bindings[set][binding.binding] = binding;
    }
}
}  // namespace TTe

namespace TTe {

Shader::Shader() {}

Shader::Shader(Device *p_device, std::filesystem::path p_shader_path, VkShaderStageFlags p_descriptor_stage, VkShaderStageFlags p_next_shader_stage)
    : m_shader_path(p_shader_path), m_next_shader_stage(p_next_shader_stage), m_device(p_device) {
    m_shader_stage = getShaderStageFlagsBitFromFileName(m_shader_path);
    loadShaderGLSLCode();

    std::filesystem::path shader_file = std::filesystem::path(m_shader_path).filename();
    std::filesystem::path shader_folder = std::filesystem::path(m_shader_path).parent_path();

    if (!std::ifstream((std::filesystem::path(ENGINE_DIR) / shader_folder / "spirv" / shader_file).concat(".spv")).good() ||
        getSavedHash() != getShaderSourceCodeHash()) {
        compileToSPIRV();
    }

    loadShaderSPVCode();
    createDescriptorSetLayout(p_descriptor_stage);
    createPushConstant(p_descriptor_stage);
    createShaderInfo();

    // shaderCode.clear();
    m_shader_source_code.clear();
}

Shader::~Shader() {
    if (m_shader != VK_NULL_HANDLE) {
        vkDestroyShaderEXT(*m_device, m_shader, nullptr);
    }
    if (m_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(*m_device, m_shader_module, nullptr);
    }
}

Shader::Shader(Shader &&other) {
    m_shader = other.m_shader;
    m_shader_module = other.m_shader_module;
    m_shader_create_info = other.m_shader_create_info;
    m_push_constants = other.m_push_constants;
    m_shader_path = other.m_shader_path;
    m_next_shader_stage = other.m_next_shader_stage;
    m_shader_stage = other.m_shader_stage;
    m_device = other.m_device;
    m_shader_code = std::move(other.m_shader_code);
    m_descriptors_set_layout = std::move(other.m_descriptors_set_layout);
    m_compute_work_group_size = other.m_compute_work_group_size;
    other.m_shader = VK_NULL_HANDLE;
    other.m_shader_module = VK_NULL_HANDLE;
}

Shader &Shader::operator=(Shader &&other) {
    if (this != &other) {
        if (m_shader != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(*m_device, m_shader, nullptr);
        }
        if (m_shader_module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(*m_device, m_shader_module, nullptr);
        }
        m_shader = other.m_shader;
        m_shader_module = other.m_shader_module;
        m_shader_create_info = other.m_shader_create_info;
        m_push_constants = other.m_push_constants;
        m_shader_path = other.m_shader_path;
        m_next_shader_stage = other.m_next_shader_stage;
        m_shader_stage = other.m_shader_stage;
        m_device = other.m_device;
        m_shader_code = std::move(other.m_shader_code);
        m_descriptors_set_layout = std::move(other.m_descriptors_set_layout);
        m_compute_work_group_size = other.m_compute_work_group_size;
        other.m_shader = VK_NULL_HANDLE;
        other.m_shader_module = VK_NULL_HANDLE;
    }
    return *this;
}

void Shader::loadShaderSPVCode() {
    std::filesystem::path shader_file = std::filesystem::path(m_shader_path).filename();
    std::filesystem::path shader_folder = std::filesystem::path(m_shader_path).parent_path();

    m_shader_code.clear();
    std::filesystem::path m_shader_path = std::filesystem::path(ENGINE_DIR) / shader_folder / "spirv" / shader_file;
    m_shader_path.concat(".spv");
    std::ifstream file{m_shader_path, std::ios::ate | std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error(
            "failed to open file :" / std::filesystem::path(ENGINE_DIR) / shader_folder / "spirv" / shader_file / ".spv");
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    m_shader_code.resize(file_size / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char *>(m_shader_code.data()), file_size);
    file.close();
    m_shader_create_info.codeSize = m_shader_code.size() * sizeof(m_shader_code[0]);
    m_shader_create_info.pCode = m_shader_code.data();
}

void Shader::loadShaderGLSLCode() {
    std::ifstream file{ENGINE_DIR / m_shader_path, std::ios::ate};
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file :" / std::filesystem::path(ENGINE_DIR) / m_shader_path);
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    m_shader_source_code.resize(file_size);
    file.seekg(0);
    file.read(m_shader_source_code.data(), file_size);
    file.close();

    // if last is not \0 add it
    if (m_shader_source_code.back() != '\0') {
        m_shader_source_code.push_back('\0');
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
    input.stage = getGLSLangStageFromShaderStage(m_shader_stage);
    input.client = GLSLANG_CLIENT_VULKAN;
    input.client_version = GLSLANG_TARGET_VULKAN_1_3;
    input.target_language = GLSLANG_TARGET_SPV;
    input.target_language_version = GLSLANG_TARGET_SPV_1_6;
    input.code = m_shader_source_code.data();
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
        printf("GLSL preprocessing failed %s\n", m_shader_path.c_str());
        printf("%s\n", glslang_shader_get_info_log(shader));
        printf("%s\n", glslang_shader_get_info_debug_log(shader));
        printf("%s\n", input.code);
        glslang_shader_delete(shader);
        throw std::runtime_error("GLSL preprocessing failed");
    }

    if (!glslang_shader_parse(shader, &input)) {
        printf("GLSL parsing failed %s\n", m_shader_path.c_str());
        printf("%s\n", glslang_shader_get_info_log(shader));
        printf("%s\n", glslang_shader_get_info_debug_log(shader));
        printf("%s\n", glslang_shader_get_preprocessed_code(shader));
        glslang_shader_delete(shader);
        throw std::runtime_error("GLSL parsing failed");
    }

    glslang_program_t *program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        printf("GLSL linking failed %s\n", m_shader_path.c_str());
        printf("%s\n", glslang_program_get_info_log(program));
        printf("%s\n", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        throw std::runtime_error("GLSL linking failed");
    }

    glslang_spv_options_t spv_options = {
        .generate_debug_info = false,
        .disable_optimizer = false,
        .optimize_size = false,
        .disassemble = false,
        .validate = false,
    };

    glslang_program_SPIRV_generate_with_options(program, input.stage, &spv_options);

    bin.size = glslang_program_SPIRV_get_size(program);
    bin.words.resize(bin.size);
    glslang_program_SPIRV_get(program, bin.words.data());

    const char *spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages) printf("(%s) %s\b", m_shader_path.c_str(), spirv_messages);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    std::filesystem::path shader_file = std::filesystem::path(m_shader_path).filename();
    std::filesystem::path shader_folder = std::filesystem::path(m_shader_path).parent_path();

    // write SPIR-V binary to file
    std::ofstream outFile((std::filesystem::path(ENGINE_DIR) / shader_folder / "spirv" / shader_file).concat(".spv"), std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error(
            "failed to open file for writing SPIR-V binary: " / std::filesystem::path(ENGINE_DIR) / shader_folder / "spirv" / shader_file /
            ".spv");
    }
    outFile.write(reinterpret_cast<const char *>(bin.words.data()), bin.size * sizeof(uint32_t));
    outFile.close();

    std::cout << "Shader compiled to SPIR-V: " << shader_file << std::endl;
    saveHash();
}

std::string Shader::getShaderSourceCodeHash() {
    Chocobo1::MD5 md5;

    md5.addData(m_shader_source_code.data(), m_shader_source_code.size());
    return md5.finalize().toString();
}

void Shader::saveHash() {
    std::string hash = getShaderSourceCodeHash();

    std::filesystem::path shader_file = std::filesystem::path(m_shader_path).filename();
    std::filesystem::path shader_folder = std::filesystem::path(m_shader_path).parent_path();

    std::ofstream hash_file((std::filesystem::path(ENGINE_DIR) / shader_folder / "hash" / shader_file).concat(".hash"), std::ios::binary);
    if (!hash_file.is_open()) {
        throw std::runtime_error(
            "failed to open file for writing hash: " /
            (std::filesystem::path(ENGINE_DIR) / shader_folder / "hash" / shader_file).concat(".hash"));
    }
    hash_file.write(hash.data(), hash.size());
    hash_file.close();
}

std::string Shader::getSavedHash() {
    std::filesystem::path shader_file = std::filesystem::path(m_shader_path).filename();
    std::filesystem::path shader_folder = std::filesystem::path(m_shader_path).parent_path();

    std::ifstream hash_file((std::filesystem::path(ENGINE_DIR) / shader_folder / "hash" / shader_file).concat(".hash"), std::ios::binary);
    if (!hash_file.is_open()) {
        throw std::runtime_error(
            "failed to open file for reading hash: " /
            (std::filesystem::path(ENGINE_DIR) / shader_folder / "hash" / shader_file).concat(".hash"));
    }
    std::string hash;
    hash_file.seekg(0, std::ios::end);
    hash.resize(hash_file.tellg());
    hash_file.seekg(0, std::ios::beg);
    hash_file.read(hash.data(), hash.size());
    hash_file.close();
    return hash;
}

void Shader::createDescriptorSetLayout(VkShaderStageFlags p_descriptor_stage) {
    std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> set_bindings;
    spirv_cross::Compiler comp(m_shader_code);
    spirv_cross::ShaderResources res = comp.get_shader_resources();

    comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 0);
    addSamplerBinding(res.separate_samplers, comp, set_bindings, p_descriptor_stage);
    addSampledImagesBinding(res.sampled_images, comp, set_bindings, p_descriptor_stage);
    addSeparateImagesBinding(res.separate_images, comp, set_bindings, p_descriptor_stage);
    addStorageImagesBinding(res.storage_images, comp, set_bindings, p_descriptor_stage);
    addUniformTexelBufferBinding(res.separate_images, comp, set_bindings, p_descriptor_stage);
    addStorageTexelBufferBinding(res.storage_images, comp, set_bindings, p_descriptor_stage);
    addUniformBufferBinding(res.uniform_buffers, comp, set_bindings, p_descriptor_stage);
    addStorageBufferBinding(res.storage_buffers, comp, set_bindings, p_descriptor_stage);
    addAccelerationStructureBinding(res.acceleration_structures, comp, set_bindings, p_descriptor_stage);
    if (m_shader_stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        m_compute_work_group_size.width = comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 0);
        m_compute_work_group_size.height = comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 1);
        m_compute_work_group_size.depth = comp.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 2);
    }

    for (auto &descriptor_set : set_bindings) {
        m_descriptors_set_layout.push_back(DescriptorSetLayout::createDescriptorSetLayout(m_device, descriptor_set.second, descriptor_set.first));
    }
}

void Shader::createPushConstant(VkShaderStageFlags p_descriptor_stage) {
    spirv_cross::Compiler comp(m_shader_code);

    m_push_constants = make<VkPushConstantRange>();
    m_push_constants.offset = 0;
    m_push_constants.stageFlags = p_descriptor_stage;
    auto ranges = comp.get_shader_resources().push_constant_buffers;
    for (auto &res : ranges) {
        auto &m_type = comp.get_type(res.base_type_id);
        uint32_t size = comp.get_declared_struct_size(m_type);
        m_push_constants.size = size;
    }
}

void Shader::createShaderInfo() {
    // listDescriptor = new VkDescriptorSetLayout[descriptorsSetLayout.size()];
    // for (int i = 0; i < descriptorsSetLayout.size(); i++) {
    //     listDescriptor[i] = *descriptorsSetLayout[i];
    // }

    m_shader_create_info = make<VkShaderCreateInfoEXT>();
    m_shader_create_info.flags = 0;
    m_shader_create_info.stage = m_shader_stage;
    m_shader_create_info.nextStage = m_next_shader_stage;
    m_shader_create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    m_shader_create_info.codeSize = m_shader_code.size() * sizeof(m_shader_code[0]);
    m_shader_create_info.pCode = m_shader_code.data();
    m_shader_create_info.pName = "main";
    m_shader_create_info.setLayoutCount = m_descriptors_set_layout.size();
    // m_shader_create_info.pSetLayouts = listDescriptor;
    m_shader_create_info.pushConstantRangeCount = (m_push_constants.size > 0) ? 1 : 0;
    m_shader_create_info.pPushConstantRanges = (m_push_constants.size > 0) ? &m_push_constants : nullptr;
}

// Creation of shader module for Ray-Tracing
void Shader::createShaderModule() {
    auto module_create_info = make<VkShaderModuleCreateInfo>();
    module_create_info.codeSize = m_shader_code.size() * sizeof(m_shader_code[0]);
    module_create_info.pCode = m_shader_code.data();
    auto res = vkCreateShaderModule(*m_device, &module_create_info, NULL, &m_shader_module);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }
}

void Shader::buildShader() {
    std::vector<VkDescriptorSetLayout> list_descriptor;
    for (size_t i = 0; i < m_descriptors_set_layout.size(); i++) {
        list_descriptor.push_back(*m_descriptors_set_layout[i]);
    }
    m_shader_create_info.pSetLayouts = list_descriptor.data();

    VkResult result = vkCreateShadersEXT(*m_device, 1, &m_shader_create_info, nullptr, &m_shader);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to build shader");
    }
}

void Shader::buildLinkedShaders(Device *p_device, std::vector<Shader *> &p_shaders) {
    std::vector<VkShaderCreateInfoEXT> shaders_create_infos;
    std::vector<std::vector<VkDescriptorSetLayout>> list_descriptor(p_shaders.size());
    for (size_t i = 0; i < p_shaders.size(); i++) {
        shaders_create_infos.push_back(p_shaders[i]->getShaderCreateInfo());
        shaders_create_infos[i].flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
        for (size_t j = 0; j < p_shaders[i]->getDescriptorsSetLayout().size(); j++) {
            list_descriptor[i].push_back(*p_shaders[i]->getDescriptorsSetLayout()[j]);
        }
        shaders_create_infos[i].setLayoutCount = list_descriptor[i].size();
        shaders_create_infos[i].pSetLayouts = list_descriptor[i].data();
    }

    std::vector<VkShaderEXT> shader_handler(p_shaders.size());
    VkResult result = vkCreateShadersEXT(*p_device, shaders_create_infos.size(), shaders_create_infos.data(), nullptr, shader_handler.data());

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader");
    }
    for (size_t i = 0; i < shader_handler.size(); i++) {
        p_shaders[i]->setShaderHandler(shader_handler[i]);
    }
}

constexpr unsigned int str2int(const char *str, int h = 0) { return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h]; }

VkShaderStageFlagBits Shader::getShaderStageFlagsBitFromFileName(std::filesystem::path p_shader_file) {
    switch (str2int(p_shader_file.extension().c_str())) {
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
        case (str2int(".task")):
            return VK_SHADER_STAGE_TASK_BIT_EXT;
            break;
        case (str2int(".mesh")):
            return VK_SHADER_STAGE_MESH_BIT_EXT;
            break;
        default:
            throw std::runtime_error("not supported shader m_type");
    }
}

glslang_stage_t Shader::getGLSLangStageFromShaderStage(VkShaderStageFlagBits p_shader_stage) const {
    switch (p_shader_stage) {
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
