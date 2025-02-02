
#include "descriptorSetLayout.hpp"



#include <memory>
#include <vector>

#include "../structs_vk.hpp"
#include "../utils.hpp"

namespace TTe {
std::unordered_map<std::vector<uint32_t>, std::weak_ptr<DescriptorSetLayout>> DescriptorSetLayout::descriptorSetLayoutCache;

DescriptorSetLayout::DescriptorSetLayout(
    Device *device, std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings, std::vector<uint32_t> id)
    : device(device), layoutBindings(layoutBindings), id(id) {
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
    descriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    descriptorSetLayoutInfo.pNext = &extendedInfo;

    if (vkCreateDescriptorSetLayout(*device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptorSetLayout");
    }
    getLayoutSizeAndOffsets();
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
    descriptorSetLayoutCache.erase(id);
}

std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::createDescriptorSetLayout(
    Device *device, std::map<uint32_t, VkDescriptorSetLayoutBinding> layoutBindings, uint32_t setid) {
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
    for (auto &binding : layoutBindings) {
        VkDeviceSize offset = 0;
        vkGetDescriptorSetLayoutBindingOffsetEXT(*device, descriptorSetLayout, binding.first, &offset);
        layoutOffsets[binding.first] = offset;
    }
}

const size_t &DescriptorSetLayout::getSizeOfDescriptorType(VkDescriptorType descriptorType) {
    switch (descriptorType) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return device->getDeviceDescProps().samplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return device->getDeviceDescProps().combinedImageSamplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return device->getDeviceDescProps().sampledImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return device->getDeviceDescProps().storageImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return device->getDeviceDescProps().uniformTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return device->getDeviceDescProps().storageTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return device->getDeviceDescProps().uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return device->getDeviceDescProps().storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return device->getDeviceDescProps().uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return device->getDeviceDescProps().storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return device->getDeviceDescProps().inputAttachmentDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return device->getDeviceDescProps().uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return device->getDeviceDescProps().accelerationStructureDescriptorSize;

        default:
            throw std::runtime_error("Unknown descriptor type");
    }
}

}  // namespace TTe