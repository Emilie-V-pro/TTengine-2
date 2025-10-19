#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../device.hpp"
#include "volk.h"

namespace TTe {
class DescriptorSetLayout {
   public:
    DescriptorSetLayout(Device *p_device, std::map<uint32_t, VkDescriptorSetLayoutBinding> p_layout_bindings, std::vector<uint32_t> p_id);

    static std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
        Device *p_device,
        std::map<uint32_t, VkDescriptorSetLayoutBinding> p_layout_bindings,
        uint32_t p_set_id);
    
    // delete copy constructor
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    // move constructor
    DescriptorSetLayout(DescriptorSetLayout &&other);
    DescriptorSetLayout &operator=(DescriptorSetLayout &&other);

    ~DescriptorSetLayout();
    
    // Getters
    operator const VkDescriptorSetLayout&() const { return m_descriptor_set_layout; }
    
    std::map<uint32_t, VkDescriptorSetLayoutBinding> getLayoutBindings() const { return m_layout_bindings; }
    std::vector<uint32_t> getId() const { return m_id; }
    const VkDeviceSize& getLayoutSize() const { return m_layout_size; }
    std::unordered_map<uint32_t, VkDeviceSize> &getLayoutOffsets() { return m_layout_offsets; }
    
    
    const size_t &getSizeOfDescriptorType(VkDescriptorType p_descriptor_type);
    const size_t &getSizeOfDescriptorType(uint32_t p_binding) { return getSizeOfDescriptorType(m_layout_bindings[p_binding].descriptorType); }

   private:

    static std::unordered_map<std::vector<uint32_t>, std::weak_ptr<DescriptorSetLayout>> s_descriptor_set_layout_cache;

    void getLayoutSizeAndOffsets();
    
    Device *m_device;

    VkDescriptorSetLayout m_descriptor_set_layout;

    std::map<uint32_t, VkDescriptorSetLayoutBinding> m_layout_bindings;
    std::vector<uint32_t> m_id;

    VkDeviceSize m_layout_size = 0;
    std::unordered_map<uint32_t, VkDeviceSize> m_layout_offsets;
};
}  // namespace TTe