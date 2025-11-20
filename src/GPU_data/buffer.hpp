#pragma once

#include <volk.h>

#include <atomic>
#include <cstdint>
#include <memory>

#include "../commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "structs_vk.hpp"

namespace TTe {

class Buffer : public CmdBufferRessource {
   public:
    enum struct BufferType { GPU_ONLY, STAGING, READBACK, DYNAMIC, OTHER };
    // Constructors
    Buffer(
        Device* p_device,
        VkDeviceSize p_instance_size,
        uint32_t p_instance_count,
        VkBufferUsageFlags p_usage,
        BufferType p_buffer_type,
        VkMemoryPropertyFlags p_required_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Buffer();
    // Destructor
    ~Buffer();

    // Copy and move constructors
    Buffer(const Buffer& other);
    Buffer& operator=(const Buffer& other);
    Buffer(Buffer&& other);
    Buffer& operator=(Buffer&& other);

    operator VkBuffer() const { return m_vk_buffer; }
    operator uint64_t() const { return getBufferDeviceAddress(); }
    operator VkDescriptorAddressInfoEXT() const {
        auto address_info = make<VkDescriptorAddressInfoEXT>();
        address_info.address = getBufferDeviceAddress();
        address_info.range = m_total_size;
        return address_info;
    }
    
    VkDeviceAddress getBufferDeviceAddress(uint32_t offset = 0) const;

    uint32_t getInstancesCount() const { return m_instance_count; }

    BufferType getType() const { return m_type; }

    void* mapMemory()  {
        if(m_mapped_memory == nullptr)
            vmaMapMemory(m_device->getAllocator(), m_allocation, &m_mapped_memory);
        return m_mapped_memory;
    };

    void unmapMemory() { 
        vmaUnmapMemory(m_device->getAllocator(), m_allocation); 
        m_mapped_memory = nullptr;
        }


    void writeToBuffer(void* p_data, VkDeviceSize p_size = VK_WHOLE_SIZE, VkDeviceSize p_offset = 0);
    void readFromBuffer(void* p_data, VkDeviceSize p_size = VK_WHOLE_SIZE, VkDeviceSize p_offset = 0);

    void copyToImage(
        Device* p_device, VkImage p_image, uint32_t p_width, uint32_t p_height, uint32_t p_layer = 1, CommandBuffer* p_ext_cmd_buffer = nullptr);
    
    void copyFromImage(
        Device* p_device, VkImage p_image, uint32_t p_width, uint32_t p_height, uint32_t p_layer = 1, VkImageAspectFlags p_aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, CommandBuffer* p_extCmdBuffer = nullptr);

    static void copyBuffer(
        Device* p_device,
        const Buffer& p_src_buffer,
        const Buffer& p_dst_buffer,
        CommandBuffer* p_cmd_buffer = nullptr,
        VkDeviceSize p_size = VK_WHOLE_SIZE,
        VkDeviceSize p_src_offset = 0,
        VkDeviceSize p_dst_offset = 0);

    void addBufferMemoryBarrier(const CommandBuffer& p_ext_cmd_buffer, VkPipelineStageFlags p_src_stage_mask, VkPipelineStageFlags p_dst_stage_mask);

    void transferQueueOwnership(const CommandBuffer& p_ext_cmd_buffer, uint32_t p_queue_index);




   private:
    VkBufferUsageFlags getBufferUsageFlags(BufferType p_buffer_type) const;
    VmaAllocationCreateFlags getAllocationFlags(BufferType p_buffer_type) const;
    void destruction();

    VmaAllocationCreateInfo m_alloc_info = {};
    VkBufferCreateInfo m_buffer_info = {};
    BufferType m_type = BufferType::GPU_ONLY;

    VkDeviceSize m_instance_size = 0;
    uint32_t m_instance_count = 0;
    VkDeviceSize m_total_size = 0;

    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkBuffer m_vk_buffer = VK_NULL_HANDLE;
    Device* m_device = nullptr;
    void *m_mapped_memory = nullptr;

    // TODO REMPLACER PAR MUTABLE
    std::atomic<std::shared_ptr<int>> m_ref_count;
};
}  // namespace TTe