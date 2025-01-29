#pragma once
#include <cstdint>
#include <memory>

#include "../buffer.hpp"
#include "descriptorSetLayout.hpp"
#include "volk.h"

namespace TTe {

struct DescriptorInfo {
    VkDeviceSize layoutOffset;
    VkDeviceSize layoutSize;
    VkDescriptorSetLayout setLayout;
    VkDeviceOrHostAddressConstKHR bufferDeviceAddress;
};

class DescriptorSet {
   public:
    // Constructor
    DescriptorSet(Device *device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout);

    // Destructor
    ~DescriptorSet();

    // Copy and move constructors
    DescriptorSet(DescriptorSet &other);
    DescriptorSet &operator=(DescriptorSet &other);
    DescriptorSet(DescriptorSet &&other);
    DescriptorSet &operator=(DescriptorSet &&other);


    // Functions
    void updateToGPU();
    void writeSamplerDescriptor(uint32_t binding, VkSampler sampler);
    void writeBufferDescriptor(uint32_t binding, VkDescriptorAddressInfoEXT *bufferInfo);
    void writeImageDescriptor(uint32_t binding, VkDescriptorImageInfo *imageInfo);
    void writeAccelerationStructureDescriptor(uint32_t binding, VkDeviceAddress *accelerationStructureAddress);

    void writeSamplersDescriptor(uint32_t binding, std::vector<VkSampler> *samplers);
    void writeBuffersDescriptor(uint32_t binding, std::vector<VkDescriptorAddressInfoEXT> *buffersInfo);
    void writeImagesDescriptor(uint32_t binding, std::vector<VkDescriptorImageInfo> *imagesInfo);
    void writeAccelerationStructuresDescriptor(uint32_t binding, std::vector<VkDeviceAddress> *accelerationStructuresAddresses);

    static void bindDescriptorSet(
        CommandBuffer cmdBuffer,
        std::vector<VkDescriptorSet *> &descriptorSets,
        VkPipelineLayout pipelineLayout,
        VkPipelineBindPoint bindPoint,
        uint32_t firstIndex = 0);

   private:
    Buffer descriptor_buffer;
    uint64_t descriptor_buffer_address = 0;

    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = nullptr;
    VkDescriptorGetInfoEXT descriptorInfo;

    Device *device;
};
}  // namespace TTe