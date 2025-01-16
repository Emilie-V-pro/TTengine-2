#pragma once

#include <volk.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>

#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"

namespace TTe {

class Buffer {

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

    // Destructor
    ~Buffer();

    // Copy and move constructors
    Buffer(const Buffer& other);
    Buffer& operator=(const Buffer& other);
    Buffer(Buffer&& other);
    Buffer& operator=(Buffer&& other);


    operator VkBuffer() const { return vk_buffer; }

   private:
    VkBufferUsageFlags getBufferUsageFlags(BufferType bufferType) const;
    VmaAllocationCreateFlags getAllocationFlags(BufferType bufferType) const;

    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    
    static void copyBuffer(
        Device* device,
        const Buffer & src_buffer,
        const Buffer & dst_buffer,
        CommandBuffer* cmdBuffer = nullptr,
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize src_offset = 0,
        VkDeviceSize dst_offset = 0);

    VmaAllocationCreateInfo allocInfo = {};
    VkBufferCreateInfo bufferInfo = {};

    VkDeviceSize instance_size = 0;
    uint32_t instance_count = 0;
    VkDeviceSize total_size = 0;

    VmaAllocation allocation = VK_NULL_HANDLE;
    VkBuffer vk_buffer = VK_NULL_HANDLE;
    VkDeviceMemory vk_memory = VK_NULL_HANDLE;
    Device* device = nullptr;
};
}  // namespace TTe