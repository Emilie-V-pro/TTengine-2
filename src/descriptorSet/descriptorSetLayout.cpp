
#include "descriptorSetLayout.hpp"

#include <memory>
#include <vector>
#include "../structs_vk.hpp"
#include "../utils.hpp"

namespace TTe {
std::unordered_map<std::vector<uint32_t>, std::weak_ptr<DescriptorSetLayout>> DescriptorSetLayout::descriptorSetLayoutCache;

DescriptorSetLayout::DescriptorSetLayout(
    Device *device,
    std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings,
    std::vector<uint32_t> id)
    : device(device), layoutBindings(layoutBindings), id(id){
    std::vector<VkDescriptorSetLayoutBinding> bindingsVector;

    std::vector<VkDescriptorBindingFlags> binsFlag;

    for (auto &binding : layoutBindings) {
        bindingsVector.push_back(binding.second);
        binsFlag.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT);
    }


    auto extendedInfo = make<VkDescriptorSetLayoutBindingFlagsCreateInfo>();

    extendedInfo.bindingCount = binsFlag.size();
    extendedInfo.pBindingFlags = binsFlag.data();

    auto descriptorSetLayoutInfo = make<VkDescriptorSetLayoutCreateInfo>();
    descriptorSetLayoutInfo.bindingCount = layoutBindings.size();
    descriptorSetLayoutInfo.pBindings = bindingsVector.data();
    descriptorSetLayoutInfo.pNext = &extendedInfo;

    if (vkCreateDescriptorSetLayout(*device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptorSetLayout");
    }
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
    descriptorSetLayoutCache.erase(id);
}

std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::createDescriptorSetLayout(
    Device *device,
        std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings,
        uint32_t setid) {
    std::shared_ptr<DescriptorSetLayout> returnValue;
    std::vector<uint32_t> id;
    id.push_back(setid);
    for (auto &binding : layoutBindings) {
        id.push_back(binding.second.binding);
        id.push_back(binding.second.descriptorCount);

        id.push_back(binding.second.stageFlags);
        id.push_back(binding.second.descriptorType);
    }
    if (descriptorSetLayoutCache.find(id) == descriptorSetLayoutCache.end()) {
        std::shared_ptr<DescriptorSetLayout> returnValue = std::make_shared<DescriptorSetLayout>(device, layoutBindings, id);
        descriptorSetLayoutCache[id] = returnValue;
        return returnValue;
    }

    return descriptorSetLayoutCache[id].lock();
}

void DescriptorSetLayout::getLayoutSizeAndOffsets() {
    vkGetDescriptorSetLayoutSizeEXT(*device, descriptorSetLayout, &layoutSize);
    layoutSize = alignedVkSize(layoutSize, device->getDeviceDescProps().descriptorBufferOffsetAlignment);
    for(auto &binding : layoutBindings) {
        VkDeviceSize offset = 0;
        vkGetDescriptorSetLayoutBindingOffsetEXT(*device, descriptorSetLayout, binding.first, &offset);
        layoutOffsets[binding.first] =  offset;
    }
}

}  // namespace vk_stage