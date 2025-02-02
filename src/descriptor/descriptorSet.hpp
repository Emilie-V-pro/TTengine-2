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
    DescriptorSet() {};
    DescriptorSet(Device *device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout);

    // Destructor
    ~DescriptorSet();

    // Copy and move constructors
    DescriptorSet(DescriptorSet &other);
    DescriptorSet &operator=(DescriptorSet &other);
    DescriptorSet(DescriptorSet &&other);
    DescriptorSet &operator=(DescriptorSet &&other);


    // Functions
    void writeSamplerDescriptor(uint32_t binding, const VkSampler &sampler);
    void writeBufferDescriptor(uint32_t binding, const VkDescriptorAddressInfoEXT &bufferInfo);
    void writeImageDescriptor(uint32_t binding, const VkDescriptorImageInfo &imageInfo);
    void writeAccelerationStructureDescriptor(uint32_t binding, const VkDeviceAddress &accelerationStructureAddress);

    void writeSamplersDescriptor(uint32_t binding, const std::vector<VkSampler> &samplers);
    void writeBuffersDescriptor(uint32_t binding, const std::vector<VkDescriptorAddressInfoEXT> &buffersInfo);
    void writeImagesDescriptor(uint32_t binding, const std::vector<VkDescriptorImageInfo> &imagesInfo);

    static void bindDescriptorSet(
        const CommandBuffer &cmdBuffer,
        std::vector<DescriptorSet *> &descriptorSets,
        VkPipelineLayout pipelineLayout,
        VkPipelineBindPoint bindPoint,
        uint32_t firstIndex = 0);

   private:
    Buffer descriptor_buffer;
    uint64_t descriptor_buffer_address = 0;

    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = nullptr;
    VkDescriptorGetInfoEXT descriptorInfo = {};

    Device *device;
};
}  // namespace TTe