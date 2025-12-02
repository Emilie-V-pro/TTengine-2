
#include "device.hpp"
#include <vulkan/vulkan_core.h>

#include <iostream>

#include "VkBootstrap.h"
#include "commandBuffer/commandPool_handler.hpp"
#include "structs_vk.hpp"

#define VMA_IMPLEMENTATION
#include <volk.h>

#include "vk_mem_alloc.h"
namespace TTe {

Device::Device(Window &p_window) {
    createInstance();
    selectPhysicalDevice(p_window);
    createLogicialDevice();
    initVMA();
    queryPhysicalDeviceProperties();
}

void Device::createInstance() {
    vkb::InstanceBuilder instance_builder;
    instance_builder.set_app_name("TTEngine 2.0").set_engine_name("TTEngine 2.0").require_api_version(1, 4, 0);

    if (enableValidationLayers) {
        instance_builder.request_validation_layers()
            .use_default_debug_messenger();  //.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
    }
    auto instance_builder_return = instance_builder.build();

    if (!instance_builder_return) {
        throw std::runtime_error(instance_builder_return.error().message());
    }
    m_vkb_instance = std::move(instance_builder_return.value());
    volkInitialize();
    volkLoadInstance(m_vkb_instance.instance);
    m_vk_instance = m_vkb_instance.instance;
}

void Device::selectPhysicalDevice(Window &p_window) {
    this->m_surface = p_window.createSurface(m_vkb_instance);
    std::cout << "m_surface created" << std::endl;
    vkb::PhysicalDeviceSelector phys_device_selector(m_vkb_instance);
    phys_device_selector.set_surface(m_surface);
    setRequiredExtensions(phys_device_selector);
    setRequiredFeatures(phys_device_selector);
    phys_device_selector.prefer_gpu_device_type(vkb::PreferredDeviceType::integrated);
    auto physical_device_selector_return = phys_device_selector.select();
    std::cout << physical_device_selector_return->name << "\n";
    if (!physical_device_selector_return) {
        throw std::runtime_error(physical_device_selector_return.error().message());
    }
    m_vkb_physical_device = std::move(physical_device_selector_return.value());
}

void Device::createLogicialDevice() {
    vkb::DeviceBuilder device_builder{m_vkb_physical_device};
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        throw std::runtime_error(dev_ret.error().message());
    }
    m_vkb_device = std::move(dev_ret.value());

    m_render_queue_family_index = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    m_compute_queue_family_index = m_vkb_device.get_queue_index(vkb::QueueType::compute).value();
    m_transfer_queue_family_index = m_vkb_device.get_queue_index(vkb::QueueType::transfer).value();
    m_present_queue = m_vkb_device.get_queue(vkb::QueueType::present).value();
    vkGetDeviceQueue(m_vkb_device.device, m_render_queue_family_index, 0, &m_render_queue);
    vkGetDeviceQueue(m_vkb_device.device, m_compute_queue_family_index, 0, &m_compute_queue);
    vkGetDeviceQueue(m_vkb_device.device, m_transfer_queue_family_index, 0, &m_transfer_queue);

    m_vk_device = m_vkb_device.device;
}

void Device::initVMA() {
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = m_vkb_physical_device.physical_device;
    allocator_info.device = *this;
    allocator_info.instance = m_vkb_instance.instance;
    allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;

    m_vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    m_vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

    m_vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
    m_vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
    m_vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
    m_vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
    m_vma_vulkan_func.vkCreateImage = vkCreateImage;
    m_vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
    m_vma_vulkan_func.vkDestroyImage = vkDestroyImage;
    m_vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    m_vma_vulkan_func.vkFreeMemory = vkFreeMemory;
    m_vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    m_vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    m_vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    m_vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    m_vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    m_vma_vulkan_func.vkMapMemory = vkMapMemory;
    m_vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
    m_vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

    allocator_info.pVulkanFunctions = &m_vma_vulkan_func;
    allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocator_info, &m_allocator);
}

void setRequiredExtensionsFeatures(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_obj_feature{};
    shader_obj_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT;
    shader_obj_feature.shaderObject = true;

    VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_feature{};
    descriptor_buffer_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    descriptor_buffer_feature.descriptorBuffer = true;
    descriptor_buffer_feature.descriptorBufferCaptureReplay = true;

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_feature{};
    mesh_shader_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    mesh_shader_feature.meshShader = true;
    mesh_shader_feature.taskShader = true;


    p_phys_device_selector.add_required_extension_features(shader_obj_feature);
    p_phys_device_selector.add_required_extension_features(descriptor_buffer_feature);
    p_phys_device_selector.add_required_extension_features(mesh_shader_feature);
}

// Features paramater

void setRequiredFeatures10(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    auto required_features = make<VkPhysicalDeviceFeatures>();
    required_features.samplerAnisotropy = true;
    required_features.shaderInt64 = true;
    required_features.fillModeNonSolid = true;
    required_features.wideLines = true;
    required_features.largePoints  = true;
    p_phys_device_selector.set_required_features(required_features);
}

void setRequiredFeatures11(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    auto required_features = make<VkPhysicalDeviceVulkan11Features>();
    p_phys_device_selector.set_required_features_11(required_features);
}

void setRequiredFeatures12(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    auto required_features12 = make<VkPhysicalDeviceVulkan12Features>();
    required_features12.descriptorBindingPartiallyBound = true;
    // required_features12.runtimeDescriptorArray = true;
    //  required_features12.descriptorBindingVariableDescriptorCount = true;
    required_features12.drawIndirectCount = true;
    // required_features12.samplerFilterMinmax = true;
    required_features12.bufferDeviceAddress = true;
    required_features12.descriptorIndexing = true;
    required_features12.scalarBlockLayout = true;
    required_features12.timelineSemaphore = true;
    required_features12.shaderSampledImageArrayNonUniformIndexing = true;
    
    p_phys_device_selector.set_required_features_12(required_features12);
}

void setRequiredFeatures13(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    auto required_features13 = make<VkPhysicalDeviceVulkan13Features>();
    required_features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    required_features13.dynamicRendering = true;
    required_features13.synchronization2 = true;
    required_features13.shaderDemoteToHelperInvocation = true;
    p_phys_device_selector.set_required_features_13(required_features13);
}


void Device::setRequiredFeatures(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    setRequiredExtensionsFeatures(p_phys_device_selector);
    setRequiredFeatures10(p_phys_device_selector);
    setRequiredFeatures12(p_phys_device_selector);
    setRequiredFeatures13(p_phys_device_selector);
}

void Device::setRequiredExtensions(vkb::PhysicalDeviceSelector &p_phys_device_selector) {
    p_phys_device_selector.add_required_extension("VK_EXT_shader_object");
    p_phys_device_selector.add_required_extension("VK_EXT_descriptor_buffer");
    p_phys_device_selector.add_required_extension("VK_KHR_swapchain_mutable_format");
    p_phys_device_selector.add_required_extension("VK_EXT_mesh_shader");
}

void Device::queryPhysicalDeviceProperties() {
    m_device_props_2 = make<VkPhysicalDeviceProperties2>();
    m_device_desc_props = make<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();

    m_device_props_2.pNext = &m_device_desc_props;
    vkGetPhysicalDeviceProperties2(m_vkb_physical_device.physical_device, &m_device_props_2);
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &p_candidates, VkImageTiling p_tiling, VkFormatFeatureFlags p_features) {
    for (VkFormat format : p_candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_vkb_device.physical_device, format, &props);

        if (p_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & p_features) == p_features) {
            return format;
        } else if (p_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & p_features) == p_features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

Device::~Device() {
    CommandPoolHandler::cleanUnusedPools();
    vmaDestroyAllocator(m_allocator);
    vkb::destroy_device(m_vkb_device);
    vkb::destroy_surface(m_vkb_instance, m_surface);
    vkb::destroy_instance(m_vkb_instance);

}

}  // namespace TTe