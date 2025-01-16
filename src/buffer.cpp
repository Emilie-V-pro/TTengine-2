
#include "buffer.hpp"

#include <vulkan/vulkan_core.h>

#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "structs_vk.hpp"

namespace TTe {

Buffer::Buffer(
    Device* device,
    VkDeviceSize instance_size,
    uint32_t instance_count,
    VkBufferUsageFlags usage,
    BufferType bufferType,
    VkMemoryPropertyFlags requiredProperties)
    : instance_size(instance_size), instance_count(instance_count), total_size(instance_count * instance_size), device(device) {
    bufferInfo = make<VkBufferCreateInfo>();
    bufferInfo.size = total_size;
    bufferInfo.usage = usage | getBufferUsageFlags(bufferType);

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    allocInfo = {};
    allocInfo.requiredFlags = requiredProperties;
    allocInfo.flags = getAllocationFlags(bufferType);
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &vk_buffer, &allocation, nullptr);
}

Buffer::~Buffer() {
    if (vk_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(device->getAllocator(), vk_buffer, allocation);
}

Buffer::Buffer(const Buffer& other)
    : allocInfo(other.allocInfo),
      bufferInfo(other.bufferInfo),
      instance_size(other.instance_size),
      instance_count(other.instance_count),
      total_size(instance_count * instance_size),
      device(other.device) {
    bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &vk_buffer, &allocation, nullptr);
    copyBuffer(device, other, *this);
}

Buffer& Buffer::operator=(const Buffer& other) {
    if (this != &other) {
        this->~Buffer();
        instance_size = other.instance_size;
        instance_count = other.instance_count;
        total_size = instance_count * instance_size;
        device = other.device;
        allocInfo = other.allocInfo;
        bufferInfo = other.bufferInfo;
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &vk_buffer, &allocation, nullptr);
        copyBuffer(device, other, *this);
    }
    return *this;
}

Buffer::Buffer(Buffer&& other) {
    allocInfo = other.allocInfo;
    bufferInfo = other.bufferInfo;

    instance_size = other.instance_size;
    instance_count = other.instance_count;
    total_size = instance_count * instance_size;

    allocation = other.allocation;
    vk_buffer = other.vk_buffer;
    vk_memory = other.vk_memory;
    device = other.device;

    other.vk_buffer = VK_NULL_HANDLE;
    other.vk_memory = VK_NULL_HANDLE;
    other.allocation = VK_NULL_HANDLE;
    other.device = nullptr;
}

Buffer& Buffer::operator=(Buffer&& other) {
    if (this != &other) {
        this->~Buffer();
        allocInfo = other.allocInfo;
        bufferInfo = other.bufferInfo;

        instance_size = other.instance_size;
        instance_count = other.instance_count;
        total_size = instance_count * instance_size;

        allocation = other.allocation;
        vk_buffer = other.vk_buffer;
        vk_memory = other.vk_memory;
        device = other.device;

        other.vk_buffer = VK_NULL_HANDLE;
        other.vk_memory = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.device = nullptr;
    }
    return *this;
}

VkBufferUsageFlags Buffer::getBufferUsageFlags(BufferType bufferType) const {
    switch (bufferType) {
        case BufferType::GPU_ONLY:
            return 0;
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

void Buffer::copyBuffer(
    Device* device,
    const Buffer& src_buffer,
    const Buffer& dst_buffer,
    CommandBuffer* extCmdBuffer,
    VkDeviceSize size,
    VkDeviceSize src_offset,
    VkDeviceSize dst_offset) {
    CommandBuffer* cmdBuffer = extCmdBuffer;
    if (cmdBuffer == nullptr) {
        cmdBuffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmdBuffer->beginCommandBuffer();
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = src_offset;  // Optional
    copyRegion.dstOffset = dst_offset;  // Optional
    
    copyRegion.size = (size == VK_WHOLE_SIZE) ? src_buffer.total_size : size;
    vkCmdCopyBuffer(*cmdBuffer, src_buffer, dst_buffer, 1, &copyRegion);
    if (extCmdBuffer == nullptr) {
        cmdBuffer->endCommandBuffer();
        cmdBuffer->addRessourceToDestroy(cmdBuffer);
        cmdBuffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

}  // namespace TTe