
#include "descriptorSet.hpp"


#include <cstddef>
#include <cstdint>
#include "structs_vk.hpp"

namespace TTe {

DescriptorSet::DescriptorSet(Device *p_device, std::shared_ptr<DescriptorSetLayout> p_descriptor_set_layout)
    : m_descriptor_set_layout(p_descriptor_set_layout), m_device(p_device) {
    m_descriptor_buffer = Buffer(
        m_device, m_descriptor_set_layout->getLayoutSize(), 1,
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        Buffer::BufferType::DYNAMIC);

    m_descriptor_buffer_address = m_descriptor_buffer.getBufferDeviceAddress();
}



void DescriptorSet::writeSamplerDescriptor(uint32_t p_binding, const VkSampler &p_sampler) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();

    descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;
    descriptorGetInfo.data.pSampler = &p_sampler;
    vkGetDescriptorEXT(
        *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding),
        data + m_descriptor_set_layout->getLayoutOffsets()[p_binding]);
}

void DescriptorSet::writeBufferDescriptor(uint32_t p_binding, const VkDescriptorAddressInfoEXT &p_buffer_info) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();

    descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;
    descriptorGetInfo.data.pStorageBuffer = &p_buffer_info;  // UBO/SSBO/VBO have the same m_type and well be determined by the descriptorType
    vkGetDescriptorEXT(
        *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding),
        data + m_descriptor_set_layout->getLayoutOffsets()[p_binding]);
}

void DescriptorSet::writeImageDescriptor(uint32_t p_binding, const VkDescriptorImageInfo &p_image_info) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();

    VkDescriptorImageInfo imageInfoRef = p_image_info;

    descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;
    descriptorGetInfo.data.pStorageImage =
        &imageInfoRef;  // combined image sampler/ storage image/ sampled image have the same m_type and well be determined by the descriptorType
    vkGetDescriptorEXT(
        *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding),
        data + m_descriptor_set_layout->getLayoutOffsets()[p_binding]);
}

void DescriptorSet::writeAccelerationStructureDescriptor(uint32_t p_binding, const VkDeviceAddress &p_acceleration_structure_address) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();

    descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;
    descriptorGetInfo.data.accelerationStructure = p_acceleration_structure_address;
    vkGetDescriptorEXT(
        *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding),
        data + m_descriptor_set_layout->getLayoutOffsets()[p_binding]);
}

void DescriptorSet::writeSamplersDescriptor(uint32_t p_binding, const std::vector<VkSampler> &p_samplers) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();

    descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;
    descriptorGetInfo.data.pSampler = p_samplers.data();
    vkGetDescriptorEXT(
        *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding) * p_samplers.size(),
        data + m_descriptor_set_layout->getLayoutOffsets()[p_binding]);
}

void DescriptorSet::writeBuffersDescriptor(uint32_t p_binding, const std::vector<VkDescriptorAddressInfoEXT> &p_buffers_info) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();
    
    descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;
    descriptorGetInfo.data.pStorageBuffer = p_buffers_info.data();
    vkGetDescriptorEXT(
        *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding) * p_buffers_info.size(),
        data + m_descriptor_set_layout->getLayoutOffsets()[p_binding]);
}

void DescriptorSet::writeImagesDescriptor(uint32_t p_binding, const std::vector<VkDescriptorImageInfo> &p_images_info) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)m_descriptor_buffer.mapMemory();
     descriptorGetInfo.type = m_descriptor_set_layout->getLayoutBindings()[p_binding].descriptorType;

    for(size_t i = 0; i < p_images_info.size(); i++){
        descriptorGetInfo.data.pStorageImage = &p_images_info[i];
        vkGetDescriptorEXT(
            *m_device, &descriptorGetInfo, m_descriptor_set_layout->getSizeOfDescriptorType(p_binding),
            data + m_descriptor_set_layout->getLayoutOffsets()[p_binding] + i * m_descriptor_set_layout->getSizeOfDescriptorType(p_binding));
    
    }
}

void DescriptorSet::bindDescriptorSet(
    const CommandBuffer &p_cmd_buffer,
    std::vector<DescriptorSet *> &p_descriptor_sets,
    VkPipelineLayout p_pipeline_layout,
    VkPipelineBindPoint p_bind_point,
    uint32_t p_first_index) {
    std::vector<VkDescriptorBufferBindingInfoEXT> bindingInfos(p_descriptor_sets.size());
    std::vector<uint32_t> indices(p_descriptor_sets.size());
    std::vector<VkDeviceSize> offsets(p_descriptor_sets.size());

    for (size_t i = 0; i < p_descriptor_sets.size(); i++) {
        bindingInfos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
        bindingInfos[i].pNext = nullptr;
        bindingInfos[i].address = p_descriptor_sets[i]->m_descriptor_buffer_address;
        bindingInfos[i].usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        indices[i] = i;
        offsets[i] = 0;
    }
    vkCmdBindDescriptorBuffersEXT(p_cmd_buffer, bindingInfos.size(), bindingInfos.data());

    vkCmdSetDescriptorBufferOffsetsEXT(
        p_cmd_buffer, p_bind_point, p_pipeline_layout, p_first_index, bindingInfos.size(), indices.data(), offsets.data());
}

}  // namespace TTe