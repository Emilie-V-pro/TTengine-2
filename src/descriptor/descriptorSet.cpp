
#include "descriptorSet.hpp"


#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "structs_vk.hpp"

namespace TTe {

DescriptorSet::DescriptorSet(Device *device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout)
    : descriptorSetLayout(descriptorSetLayout), device(device) {
    descriptor_buffer = Buffer(
        device, descriptorSetLayout->getLayoutSize(), 1,
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        Buffer::BufferType::DYNAMIC);

    descriptor_buffer_address = descriptor_buffer.getBufferDeviceAddress();
}



void DescriptorSet::writeSamplerDescriptor(uint32_t binding, const VkSampler &sampler) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();

    descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;
    descriptorGetInfo.data.pSampler = &sampler;
    vkGetDescriptorEXT(
        *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding),
        data + descriptorSetLayout->getLayoutOffsets()[binding]);
}

void DescriptorSet::writeBufferDescriptor(uint32_t binding, const VkDescriptorAddressInfoEXT &bufferInfo) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();

    descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;
    descriptorGetInfo.data.pStorageBuffer = &bufferInfo;  // UBO/SSBO/VBO have the same type and well be determined by the descriptorType
    vkGetDescriptorEXT(
        *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding),
        data + descriptorSetLayout->getLayoutOffsets()[binding]);
}

void DescriptorSet::writeImageDescriptor(uint32_t binding, const VkDescriptorImageInfo &imageInfo) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();

    VkDescriptorImageInfo imageInfoRef = imageInfo;

    descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;
    descriptorGetInfo.data.pStorageImage =
        &imageInfoRef;  // combined image sampler/ storage image/ sampled image have the same type and well be determined by the descriptorType
    vkGetDescriptorEXT(
        *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding),
        data + descriptorSetLayout->getLayoutOffsets()[binding]);
}

void DescriptorSet::writeAccelerationStructureDescriptor(uint32_t binding, const VkDeviceAddress &accelerationStructureAddress) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();

    descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;
    descriptorGetInfo.data.accelerationStructure = accelerationStructureAddress;
    vkGetDescriptorEXT(
        *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding),
        data + descriptorSetLayout->getLayoutOffsets()[binding]);
}

void DescriptorSet::writeSamplersDescriptor(uint32_t binding, const std::vector<VkSampler> &samplers) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();

    descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;
    descriptorGetInfo.data.pSampler = samplers.data();
    vkGetDescriptorEXT(
        *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding) * samplers.size(),
        data + descriptorSetLayout->getLayoutOffsets()[binding]);
}

void DescriptorSet::writeBuffersDescriptor(uint32_t binding, const std::vector<VkDescriptorAddressInfoEXT> &buffersInfo) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();
    
    descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;
    descriptorGetInfo.data.pStorageBuffer = buffersInfo.data();
    vkGetDescriptorEXT(
        *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding) * buffersInfo.size(),
        data + descriptorSetLayout->getLayoutOffsets()[binding]);
}

void DescriptorSet::writeImagesDescriptor(uint32_t binding, const std::vector<VkDescriptorImageInfo> &imagesInfo) {
    auto descriptorGetInfo = make<VkDescriptorGetInfoEXT>();
    char *data = (char *)descriptor_buffer.mapMemory();
     descriptorGetInfo.type = descriptorSetLayout->getLayoutBindings()[binding].descriptorType;

    for(size_t i = 0; i < imagesInfo.size(); i++){
        descriptorGetInfo.data.pStorageImage = &imagesInfo[i];
        vkGetDescriptorEXT(
            *device, &descriptorGetInfo, descriptorSetLayout->getSizeOfDescriptorType(binding),
            data + descriptorSetLayout->getLayoutOffsets()[binding] + i * descriptorSetLayout->getSizeOfDescriptorType(binding));
    
    }
}

void DescriptorSet::bindDescriptorSet(
    const CommandBuffer &cmdBuffer,
    std::vector<DescriptorSet *> &descriptorSets,
    VkPipelineLayout pipelineLayout,
    VkPipelineBindPoint bindPoint,
    uint32_t firstIndex) {
    std::vector<VkDescriptorBufferBindingInfoEXT> bindingInfos(descriptorSets.size());
    std::vector<uint32_t> indices(descriptorSets.size());
    std::vector<VkDeviceSize> offsets(descriptorSets.size());

    for (size_t i = 0; i < descriptorSets.size(); i++) {
        bindingInfos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
        bindingInfos[i].pNext = nullptr;
        bindingInfos[i].address = descriptorSets[i]->descriptor_buffer_address;
        bindingInfos[i].usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        indices[i] = i;
        offsets[i] = 0;
    }
    vkCmdBindDescriptorBuffersEXT(cmdBuffer, bindingInfos.size(), bindingInfos.data());

    vkCmdSetDescriptorBufferOffsetsEXT(
        cmdBuffer, bindPoint, pipelineLayout, firstIndex, bindingInfos.size(), indices.data(), offsets.data());
}

}  // namespace TTe