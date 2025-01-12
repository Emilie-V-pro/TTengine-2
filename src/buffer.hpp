#pragma once

#include <volk.h>
#include <vulkan/vulkan_core.h>

#include "command_buffer.hpp"
#include "device.hpp"

namespace TTe {

class Buffer {
    enum struct BufferType { GPU_ONLY, STAGING, READBACK, DYNAMIC, OTHER };

   public:
    Buffer(
        Device* device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        BufferType bufferType,
        VkMemoryPropertyFlags requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ~Buffer();

   private:
    VkBufferUsageFlags getBufferUsageFlags(BufferType bufferType) const;
    VmaAllocationCreateFlags getAllocationFlags(BufferType bufferType) const;

    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    
    static void copyBuffer(
        Device* device,
        Buffer src_buffer,
        Buffer dst_buffer,
        CommandBuffer* cmdBuffer = nullptr,
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize src_offset = 0,
        VkDeviceSize dst_offset = 0);

    VmaAllocation allocation = VK_NULL_HANDLE;
    VkBuffer vk_buffer = VK_NULL_HANDLE;
    VkDeviceMemory vk_memory = VK_NULL_HANDLE;
    Device* device = nullptr;
};
}  // namespace TTe