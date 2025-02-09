
#include "buffer.hpp"

#include <iostream>

#include "../commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "structs_vk.hpp"
#include "utils.hpp"
namespace TTe {

Buffer::Buffer(
    Device* device,
    VkDeviceSize instance_size,
    uint32_t instance_count,
    VkBufferUsageFlags usage,
    BufferType bufferType,
    VkMemoryPropertyFlags requiredProperties)
    : instance_size(instance_size), instance_count(instance_count), total_size(instance_count * instance_size), device(device) {
    refCount.store(std::make_shared<int>(1), std::memory_order_relaxed);
    bufferInfo = make<VkBufferCreateInfo>();
    bufferInfo.size = total_size;
    bufferInfo.usage = usage | getBufferUsageFlags(bufferType) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    allocInfo = {};
    allocInfo.requiredFlags = requiredProperties;
    allocInfo.flags = getAllocationFlags(bufferType);

    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocationInfo getAllocInfo;
    vmaCreateBuffer(device->getAllocator(), &bufferInfo, &allocInfo, &vk_buffer, &allocation, &getAllocInfo);
    //check if match VMA_ALLOCATION_CREATE_MAPPED_BIT
    if (bufferType == BufferType::STAGING || bufferType == BufferType::READBACK || bufferType == BufferType::DYNAMIC) {
        mappedMemory = getAllocInfo.pMappedData;
    }

}

Buffer::Buffer() {}


void Buffer::destruction(){
    if (vk_buffer != VK_NULL_HANDLE) {
        if (refCount.load(std::memory_order_relaxed) && --(*refCount.load()) == 0) {
            if (mappedMemory != nullptr) {
                vmaUnmapMemory(device->getAllocator(), allocation);
            }

            vmaDestroyBuffer(device->getAllocator(), vk_buffer, allocation);
        }
    }
}

Buffer::~Buffer() {
    destruction();
}

Buffer::Buffer(const Buffer& other)
    : allocInfo(other.allocInfo),
      bufferInfo(other.bufferInfo),
      instance_size(other.instance_size),
      instance_count(other.instance_count),
      total_size(instance_count * instance_size),
      allocation(other.allocation),
      vk_buffer(other.vk_buffer),
      vk_memory(other.vk_memory),
      device(other.device) {
    refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
    (*refCount.load())++;
}

Buffer& Buffer::operator=(const Buffer& other) {
    if (this != &other) {
        destruction();
        allocInfo = other.allocInfo;
        bufferInfo = other.bufferInfo;
        instance_size = other.instance_size;
        instance_count = other.instance_count;
        total_size = instance_count * instance_size;
        allocation = other.allocation;
        vk_buffer = other.vk_buffer;
        vk_memory = other.vk_memory;
        device = other.device;
        refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
        (*refCount.load())++;
    }
    return *this;
}

Buffer::Buffer(Buffer&& other)
    : allocInfo(other.allocInfo),
      bufferInfo(other.bufferInfo),
      instance_size(other.instance_size),
      instance_count(other.instance_count),
      total_size(instance_count * instance_size),
      allocation(other.allocation),
      vk_buffer(other.vk_buffer),
      vk_memory(other.vk_memory),
      device(other.device) {
    refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
    other.vk_buffer = VK_NULL_HANDLE;
}

Buffer& Buffer::operator=(Buffer&& other) {
    if (this != &other) {
        destruction();

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
        refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
        
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

uint64_t Buffer::getBufferDeviceAddress(uint32_t offset) const {
    auto bufferDeviceAI = make<VkBufferDeviceAddressInfo>();
    bufferDeviceAI.buffer = vk_buffer;
    return vkGetBufferDeviceAddress(*device, &bufferDeviceAI) + offset;
}

void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
    vmaCopyMemoryToAllocation(device->getAllocator(), data, allocation, offset, size);
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

void Buffer::addBufferMemoryBarrier(
    const CommandBuffer& extCmdBuffer, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask) {
    auto bufferMemoryBarrier = make<VkBufferMemoryBarrier>();
    bufferMemoryBarrier.buffer = vk_buffer;
    bufferMemoryBarrier.srcAccessMask = getFlagFromPipelineStage(srcStageMask);
    bufferMemoryBarrier.dstAccessMask = getFlagFromPipelineStage(dstStageMask);
    bufferMemoryBarrier.size = total_size;
    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.offset = 0;
    vkCmdPipelineBarrier(extCmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
}

void Buffer::transferQueueOwnership(const CommandBuffer& extCmdBuffer, uint32_t queueIndex) {
    auto bufferMemoryBarrier = make<VkBufferMemoryBarrier>();
    bufferMemoryBarrier.buffer = vk_buffer;
    bufferMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                                        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    bufferMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    bufferMemoryBarrier.size = total_size;
    bufferMemoryBarrier.srcQueueFamilyIndex = extCmdBuffer.getQueueFamilyIndex();
    bufferMemoryBarrier.dstQueueFamilyIndex = queueIndex;
    bufferMemoryBarrier.offset = 0;
    vkCmdPipelineBarrier(
        extCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0,
        nullptr);
}

void Buffer::copyToImage(Device* device, VkImage image, uint32_t width, uint32_t height, uint32_t layer, CommandBuffer* extCmdBuffer) {
    CommandBuffer* cmdBuffer = extCmdBuffer;
    if (cmdBuffer == nullptr) {
        cmdBuffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmdBuffer->beginCommandBuffer();
    }
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layer;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(*cmdBuffer, *this, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    if (extCmdBuffer == nullptr) {
        cmdBuffer->endCommandBuffer();
        cmdBuffer->addRessourceToDestroy(cmdBuffer);
        cmdBuffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

}  // namespace TTe