#pragma once

#include <volk.h>

#include <atomic>
#include <cstdint>
#include <memory>

#include "../commandBuffer/command_buffer.hpp"
#include "IRessource.hpp"
#include "device.hpp"
#include "structs_vk.hpp"

namespace TTe {

class Buffer : public vk_cmdBuffer_OBJ, public Ressource {
   public:
    enum struct BufferType { GPU_ONLY, STAGING, READBACK, DYNAMIC, OTHER };
    // Constructors
    Buffer(
        Device* device,
        VkDeviceSize instance_size,
        uint32_t instance_count,
        VkBufferUsageFlags usage,
        BufferType bufferType,
        VkMemoryPropertyFlags requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Buffer();
    // Destructor
    ~Buffer();

    // Copy and move constructors
    Buffer(const Buffer& other);
    Buffer& operator=(const Buffer& other);
    Buffer(Buffer&& other);
    Buffer& operator=(Buffer&& other);

    operator VkBuffer() const { return vk_buffer; }
    operator uint64_t() const { return getBufferDeviceAddress(); }
    operator VkDescriptorAddressInfoEXT() const {
        auto addressInfo = make<VkDescriptorAddressInfoEXT>();
        addressInfo.address = getBufferDeviceAddress();
        addressInfo.range = total_size;
        return addressInfo;
    }
    
    VkDeviceAddress getBufferDeviceAddress(uint32_t offset = 0) const;

    uint32_t getInstancesCount() const { return instance_count; }

    BufferType getType() const { return type; }

    void* mapMemory()  {
        if(mappedMemory == nullptr)
            vmaMapMemory(device->getAllocator(), allocation, &mappedMemory);
        return mappedMemory;
    };

    void unmapMemory() { 
        vmaUnmapMemory(device->getAllocator(), allocation); 
        mappedMemory = nullptr;
        }


    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void readFromBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void copyToImage(
        Device* device, VkImage image, uint32_t width, uint32_t height, uint32_t layer = 1, CommandBuffer* extCmdBuffer = nullptr);
    
    void copyFromImage(
        Device* device, VkImage image, uint32_t width, uint32_t height, uint32_t layer = 1, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, CommandBuffer* extCmdBuffer = nullptr);

    static void copyBuffer(
        Device* device,
        const Buffer& src_buffer,
        const Buffer& dst_buffer,
        CommandBuffer* cmdBuffer = nullptr,
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize src_offset = 0,
        VkDeviceSize dst_offset = 0);

    void addBufferMemoryBarrier(const CommandBuffer& extCmdBuffer, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask);

    void transferQueueOwnership(const CommandBuffer& extCmdBuffer, uint32_t queueIndex);

   private:
    VkBufferUsageFlags getBufferUsageFlags(BufferType bufferType) const;
    VmaAllocationCreateFlags getAllocationFlags(BufferType bufferType) const;
    void destruction();

    VmaAllocationCreateInfo allocInfo = {};
    VkBufferCreateInfo bufferInfo = {};
    BufferType type = BufferType::GPU_ONLY;

    VkDeviceSize instance_size = 0;
    uint32_t instance_count = 0;
    VkDeviceSize total_size = 0;

    VmaAllocation allocation = VK_NULL_HANDLE;
    VkBuffer vk_buffer = VK_NULL_HANDLE;
    Device* device = nullptr;
    void *mappedMemory = nullptr;

    // TODO REMPLACER PAR MUTABLE
    std::atomic<std::shared_ptr<int>> refCount;
};
}  // namespace TTe