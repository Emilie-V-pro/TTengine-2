#pragma once
#include <cstdint>
#include <memory>

#include "../GPU_data/buffer.hpp"
#include "descriptorSetLayout.hpp"
#include "volk.h"

namespace TTe {

struct DescriptorInfo {
    VkDeviceSize layout_offset;
    VkDeviceSize layout_size;
    VkDescriptorSetLayout set_layout;
    VkDeviceOrHostAddressConstKHR buffer_device_address;
};

class DescriptorSet {
   public:
    // Constructor
    DescriptorSet() = default;
    DescriptorSet(Device *p_device, std::shared_ptr<DescriptorSetLayout> p_descriptor_set_layout);

    // Destructor
    ~DescriptorSet() = default;

    // Copy and move constructors
    DescriptorSet(const DescriptorSet &other) = default;
    DescriptorSet &operator=(const DescriptorSet &other) = default;
    DescriptorSet(DescriptorSet &&other) = default;
    DescriptorSet &operator=(DescriptorSet &&other) = default;


    // Functions
    void writeSamplerDescriptor(uint32_t p_binding, const VkSampler &p_sampler);
    void writeBufferDescriptor(uint32_t p_binding, const VkDescriptorAddressInfoEXT &p_buffer_info);
    void writeImageDescriptor(uint32_t p_binding, const VkDescriptorImageInfo &p_image_info);
    void writeAccelerationStructureDescriptor(uint32_t binding, const VkDeviceAddress &p_acceleration_structure_address);

    void writeSamplersDescriptor(uint32_t p_binding, const std::vector<VkSampler> &p_samplers);
    void writeBuffersDescriptor(uint32_t p_binding, const std::vector<VkDescriptorAddressInfoEXT> &p_buffers_info);
    void writeImagesDescriptor(uint32_t p_binding, const std::vector<VkDescriptorImageInfo> &p_images_info);

    static void bindDescriptorSet(
        const CommandBuffer &p_cmd_buffer,
        std::vector<DescriptorSet *> &p_descriptor_sets,
        VkPipelineLayout p_pipeline_layout,
        VkPipelineBindPoint p_bind_point,
        uint32_t p_first_index = 0);

   private:
    Buffer m_descriptor_buffer;
    uint64_t m_descriptor_buffer_address = 0;

    std::shared_ptr<DescriptorSetLayout> m_descriptor_set_layout = nullptr;
    VkDescriptorGetInfoEXT m_descriptor_info = {};

    Device *m_device = nullptr;
};
}  // namespace TTe