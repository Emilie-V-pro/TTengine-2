
#include "descriptorSetLayout.hpp"



#include <memory>
#include <vector>

#include "structs_vk.hpp"
#include "utils.hpp"

namespace TTe {
std::unordered_map<std::vector<uint32_t>, std::weak_ptr<DescriptorSetLayout>> DescriptorSetLayout::s_descriptor_set_layout_cache;

DescriptorSetLayout::DescriptorSetLayout(
    Device *p_device, std::map<uint32_t, VkDescriptorSetLayoutBinding> p_layout_bindings, std::vector<uint32_t> p_id)
    : m_device(p_device), m_layout_bindings(p_layout_bindings), m_id(p_id) {
    std::vector<VkDescriptorSetLayoutBinding> bindings_vector;

    std::vector<VkDescriptorBindingFlags> bins_flag;

    for (auto &binding : m_layout_bindings) {
        bindings_vector.push_back(binding.second);
        bins_flag.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT);
    }

    auto extended_info = make<VkDescriptorSetLayoutBindingFlagsCreateInfo>();

    extended_info.bindingCount = bins_flag.size();
    extended_info.pBindingFlags = bins_flag.data();

    auto descriptor_set_layout_info = make<VkDescriptorSetLayoutCreateInfo>();
    descriptor_set_layout_info.bindingCount = m_layout_bindings.size();
    descriptor_set_layout_info.pBindings = bindings_vector.data();
    descriptor_set_layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    descriptor_set_layout_info.pNext = &extended_info;

    if (vkCreateDescriptorSetLayout(*p_device, &descriptor_set_layout_info, nullptr, &m_descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptorSetLayout");
    }
    getLayoutSizeAndOffsets();
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(*m_device, m_descriptor_set_layout, nullptr);
    s_descriptor_set_layout_cache.erase(m_id);
}

std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::createDescriptorSetLayout(
    Device *p_device, std::map<uint32_t, VkDescriptorSetLayoutBinding> p_layout_bindings, uint32_t p_set_id) {
    std::shared_ptr<DescriptorSetLayout> return_value;
    std::vector<uint32_t> id;
    id.push_back(p_set_id);
    for (auto &binding : p_layout_bindings) {
        id.push_back(binding.second.binding);
        id.push_back(binding.second.descriptorCount);

        id.push_back(binding.second.stageFlags);
        id.push_back(binding.second.descriptorType);
    }
    if (s_descriptor_set_layout_cache.find(id) == s_descriptor_set_layout_cache.end()) {
        std::shared_ptr<DescriptorSetLayout> return_value = std::make_shared<DescriptorSetLayout>(p_device, p_layout_bindings, id);
        s_descriptor_set_layout_cache[id] = return_value;
        return return_value;
    }

    return s_descriptor_set_layout_cache[id].lock();
}

void DescriptorSetLayout::getLayoutSizeAndOffsets() {
    vkGetDescriptorSetLayoutSizeEXT(*m_device, m_descriptor_set_layout, &m_layout_size);
    m_layout_size = alignedVkSize(m_layout_size, m_device->getDeviceDescProps().descriptorBufferOffsetAlignment);
    for (auto &binding : m_layout_bindings) {
        VkDeviceSize offset = 0;
        vkGetDescriptorSetLayoutBindingOffsetEXT(*m_device, m_descriptor_set_layout, binding.first, &offset);
        m_layout_offsets[binding.first] = offset;
    }
}

const size_t &DescriptorSetLayout::getSizeOfDescriptorType(VkDescriptorType p_descriptor_type) {
    switch (p_descriptor_type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return m_device->getDeviceDescProps().samplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return m_device->getDeviceDescProps().combinedImageSamplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return m_device->getDeviceDescProps().sampledImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return m_device->getDeviceDescProps().storageImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return m_device->getDeviceDescProps().uniformTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return m_device->getDeviceDescProps().storageTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return m_device->getDeviceDescProps().uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return m_device->getDeviceDescProps().storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return m_device->getDeviceDescProps().uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return m_device->getDeviceDescProps().storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return m_device->getDeviceDescProps().inputAttachmentDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return m_device->getDeviceDescProps().uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return m_device->getDeviceDescProps().accelerationStructureDescriptorSize;

        default:
            throw std::runtime_error("Unknown descriptor m_type");
    }
}

}  // namespace TTe