#pragma once
#include <cstdint>
#include <memory>

#include "../GPU_data/buffer.hpp"
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
    DescriptorSet() = default;
    DescriptorSet(Device *device, std::shared_ptr<DescriptorSetLayout> descriptorSetLayout);

    // Destructor
    ~DescriptorSet() = default;

    // Copy and move constructors
    DescriptorSet(const DescriptorSet &other) = default;
    DescriptorSet &operator=(const DescriptorSet &other) = default;
    DescriptorSet(DescriptorSet &&other) = default;
    DescriptorSet &operator=(DescriptorSet &&other) = default;


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

    Device *device = nullptr;
};
}  // namespace TTe