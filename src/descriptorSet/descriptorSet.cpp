
#include "descriptorSet.hpp"
#include "../structs_vk.hpp"
#include <utility>

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

DescriptorSet::~DescriptorSet() {}

DescriptorSet::DescriptorSet(DescriptorSet &other)
    : descriptor_buffer(other.descriptor_buffer),
      descriptor_buffer_address(descriptor_buffer.getBufferDeviceAddress()),
      descriptorSetLayout(other.descriptorSetLayout),
      descriptorInfo(other.descriptorInfo),
      device(other.device) {}

DescriptorSet::DescriptorSet(DescriptorSet &&other)
    : descriptor_buffer(std::move(other.descriptor_buffer)),
      descriptor_buffer_address(descriptor_buffer.getBufferDeviceAddress()),
      descriptorSetLayout(std::move(other.descriptorSetLayout)),
      descriptorInfo(other.descriptorInfo),
      device(other.device) {}

DescriptorSet &DescriptorSet::operator=(DescriptorSet &other) {
    if (this != &other) {
        descriptor_buffer = other.descriptor_buffer;
        descriptor_buffer_address = descriptor_buffer.getBufferDeviceAddress();
        descriptorSetLayout = other.descriptorSetLayout;
        descriptorInfo = other.descriptorInfo;
        device = other.device;
    }
    return *this;
}

DescriptorSet &DescriptorSet::operator=(DescriptorSet &&other) {
    if (this != &other) {
        descriptor_buffer = std::move(other.descriptor_buffer);
        descriptor_buffer_address = descriptor_buffer.getBufferDeviceAddress();
        descriptorSetLayout = std::move(other.descriptorSetLayout);
        descriptorInfo = other.descriptorInfo;
        device = other.device;
    }
    return *this;
}

void DescriptorSet::updateToGPU() {
    char * data = (char *)descriptor_buffer.mapMemory();
    

}

void DescriptorSet::writeSamplerDescriptor(uint32_t binding, VkSampler sampler) {
    
}
void DescriptorSet::writeBufferDescriptor(uint32_t binding, VkDescriptorAddressInfoEXT *bufferInfo) {
}
void DescriptorSet::writeImageDescriptor(uint32_t binding, VkDescriptorImageInfo *imageInfo) {
}
void DescriptorSet::writeAccelerationStructureDescriptor(uint32_t binding, VkDeviceAddress *accelerationStructureAddress) {
}
void DescriptorSet::writeSamplersDescriptor(uint32_t binding, std::vector<VkSampler> *samplers) {
}
void DescriptorSet::writeBuffersDescriptor(uint32_t binding, std::vector<VkDescriptorAddressInfoEXT> *buffersInfo) {
}
void DescriptorSet::writeImagesDescriptor(uint32_t binding, std::vector<VkDescriptorImageInfo> *imagesInfo) {
}
void DescriptorSet::writeAccelerationStructuresDescriptor(uint32_t binding, std::vector<VkDeviceAddress> *accelerationStructuresAddresses) {
}

void DescriptorSet::bindDescriptorSet(
    CommandBuffer cmdBuffer,
    std::vector<VkDescriptorSet *> &descriptorSets,
    VkPipelineLayout pipelineLayout,
    VkPipelineBindPoint bindPoint,
    uint32_t firstIndex) {}


}  // namespace TTe