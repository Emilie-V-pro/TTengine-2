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
    DescriptorSetLayout(Device *device, std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings, std::vector<uint32_t> id);

    static std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
        Device *device,
        std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings,
        uint32_t setid);
    
    // delete copy constructor
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    // move constructor
    DescriptorSetLayout(DescriptorSetLayout &&other);
    DescriptorSetLayout &operator=(DescriptorSetLayout &&other);

    ~DescriptorSetLayout();
    
    // Getters
    operator const VkDescriptorSetLayout&() const { return descriptorSetLayout; }
    
    std::map<uint32_t, VkDescriptorSetLayoutBinding> getLayoutBindings() const { return layoutBindings; }
    std::vector<uint32_t> getId() const { return id; }
    const VkDeviceSize& getLayoutSize() const { return layoutSize; }
    std::unordered_map<uint32_t, VkDeviceSize> &getLayoutOffsets() { return layoutOffsets; }
    
    
    const size_t &getSizeOfDescriptorType(VkDescriptorType descriptorType);
    const size_t &getSizeOfDescriptorType(uint32_t binding) { return getSizeOfDescriptorType(layoutBindings[binding].descriptorType); }

   private:

    static std::unordered_map<std::vector<uint32_t>, std::weak_ptr<DescriptorSetLayout>> descriptorSetLayoutCache;

    void getLayoutSizeAndOffsets();
    
    Device *device;

    VkDescriptorSetLayout descriptorSetLayout;

    std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<uint32_t> id;

    VkDeviceSize layoutSize = 0;
    std::unordered_map<uint32_t, VkDeviceSize> layoutOffsets;
};
}  // namespace TTe