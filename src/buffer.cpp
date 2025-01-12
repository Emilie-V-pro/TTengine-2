
#include "buffer.hpp"
#include <vulkan/vulkan_core.h>

#include "device.hpp"
#include "structs_vk.hpp"

namespace TTe {

Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, BufferType bufferType, VkMemoryPropertyFlags requiredProperties)
    : device(device) {
    auto bufferInfo = make<VkBufferCreateInfo>();
    bufferInfo.size = size;
    bufferInfo.usage = usage | getBufferUsageFlags(bufferType);

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = requiredProperties;
    allocInfo.flags = getAllocationFlags(bufferType);
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &vk_buffer, &allocation, nullptr);
}

VkBufferUsageFlags Buffer::getBufferUsageFlags(BufferType bufferType) const {
    switch (bufferType) {
        case BufferType::GPU_ONLY:
            return   0;
        case BufferType::STAGING:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case BufferType::READBACK:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferType::DYNAMIC:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default:
            return 0;
    }
}

VmaAllocationCreateFlags Buffer::getAllocationFlags(BufferType bufferType) const {
    switch (bufferType) {
        case BufferType::GPU_ONLY:
            return VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        case BufferType::STAGING:
            return VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        case BufferType::READBACK:
            return VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        case BufferType::DYNAMIC:
            return VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        default:
            return 0;
    }
}


void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
    vmaCopyAllocationToMemory(device->getAllocator(), allocation, offset, data, size);
}

Buffer::~Buffer() { vmaDestroyBuffer(device->getAllocator(), vk_buffer, allocation); }

}  // namespace TTe