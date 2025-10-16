#pragma once

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <stdexcept>
#include <vector>

#include "volk.h"

namespace TTe {

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;



inline VkAccessFlagBits2 getFlagFromPipelineStage(const VkPipelineStageFlags2 p_pipeline_stage) {
    VkAccessFlagBits2 return_flag = 0;
    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_NONE:
            return_flag |= VK_ACCESS_2_NONE;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
            return_flag |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT:
        case VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT:
            return_flag |= VK_ACCESS_2_INDEX_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT:
        case VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT:
            return_flag |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_UNIFORM_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
        case VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT:
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_SHADER_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_SHADER_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT:
            return_flag |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT:
            return_flag |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT:
        case VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT:
            return_flag |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT:
        case VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT:
            return_flag |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT:
        case VK_PIPELINE_STAGE_2_COPY_BIT:
        case VK_PIPELINE_STAGE_2_RESOLVE_BIT:
        case VK_PIPELINE_STAGE_2_BLIT_BIT:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR:
        case VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT:
            return_flag |= VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT:
        case VK_PIPELINE_STAGE_2_COPY_BIT:
        case VK_PIPELINE_STAGE_2_RESOLVE_BIT:
        case VK_PIPELINE_STAGE_2_BLIT_BIT:
        case VK_PIPELINE_STAGE_2_CLEAR_BIT:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR:
        case VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT:
            return_flag |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_HOST_BIT:
            return_flag |= VK_ACCESS_2_HOST_READ_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_HOST_BIT:
            return_flag |= VK_ACCESS_2_HOST_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
            break;
        default:
            break;
    }
    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            break;
        default:
            break;
    }
    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR:
            return_flag |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR:
            return_flag |= VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR:
            return_flag |= VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR:
            return_flag |= VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT:
        case VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT:
            return_flag |= VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT:
            return_flag |= VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT:
            return_flag |= VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT:
            return_flag |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV:
            return_flag |= VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV:
            return_flag |= VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR:
            return_flag |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
        case VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR:
            return_flag |= VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT:
            return_flag |= VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT:
            return_flag |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            break;
        default:
            break;
    }

    switch (p_pipeline_stage) {
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:
        case VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR:
        case VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT:
        case VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI:
        case VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI:
            return_flag |= VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;
            break;
        default:
            break;
    }
    return return_flag;
};

inline unsigned int getPixelSizeFromFormat(const VkFormat p_format) {
    switch (p_format) {
        case VK_FORMAT_UNDEFINED:
            throw std::runtime_error("Undefined image format");
            break;

        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
            return 1;
            break;

        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
            return 2;
            break;

        case VK_FORMAT_A8_UNORM_KHR:
            return 1;
            break;

        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
            return 3;
            break;

        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R16G16_S10_5_NV:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return 4;
            break;

        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
            return 6;
            break;

        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
            return 8;
            break;

        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            return 12;
            break;

        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
            return 16;
            break;

        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
            return 24;
            break;

        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return 32;
            break;

        default:
            throw std::runtime_error("Unsupported image format");
            break;
    }
}

inline VkAccessFlags getAccessFlagsFromLayout(const VkImageLayout p_layout) {
    switch (p_layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return 0;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_ACCESS_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR:
            return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return 0;
        default:
            throw std::runtime_error("Unsupported image layout");
            break;
    }
}

// from Vulkan Samples
inline VkDeviceSize alignedVkSize(const VkDeviceSize value, const VkDeviceSize alignment) { return (value + alignment - 1) & ~(alignment - 1); }

}  // namespace TTe

namespace std {
template <>
struct hash<vector<uint32_t>> {
    std::size_t operator()(std::vector<uint32_t> const& vec) const {
        std::size_t seed = vec.size();
        for (auto x : vec) {
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
}  // namespace std