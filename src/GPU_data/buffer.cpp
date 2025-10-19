
#include "buffer.hpp"

#include <fstream>
#include <iostream>

#include "../commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "structs_vk.hpp"
#include "utils.hpp"
namespace TTe {

Buffer::Buffer(
    Device* p_device,
    VkDeviceSize p__instance_size,
    uint32_t p_instance_count,
    VkBufferUsageFlags p_usage,
    BufferType p_buffer_type,
    VkMemoryPropertyFlags p_required_properties)
    : m_type(p_buffer_type),
      m_instance_size(p__instance_size),
      m_instance_count(p_instance_count),
      m_total_size(m_instance_count * m_instance_size),
      m_device(p_device) {
    m_ref_count.store(std::make_shared<int>(1), std::memory_order_relaxed);
    m_buffer_info = make<VkBufferCreateInfo>();
    m_buffer_info.size = m_total_size;
    m_buffer_info.usage = p_usage | getBufferUsageFlags(p_buffer_type);

    m_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    m_alloc_info = {};
    m_alloc_info.requiredFlags = p_required_properties;
    m_alloc_info.flags = getAllocationFlags(p_buffer_type);

    m_alloc_info.usage = (p_buffer_type == BufferType::STAGING) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO;
    VmaAllocationInfo get_alloc_info;

    // save vmaBuildStatsString to file

    VkResult res = vmaCreateBuffer(m_device->getAllocator(), &m_buffer_info, &m_alloc_info, &m_vk_buffer, &m_allocation, &get_alloc_info);
    // std::cout << res << std::endl;
    if (res != VK_SUCCESS) {
        // dump vma
        std::ofstream file("vmaStats.json");
        char* s;
        vmaBuildStatsString(m_device->getAllocator(), &s, VK_TRUE);

        file << s;
        file.close();
        vmaFreeStatsString(m_device->getAllocator(), s);
        std::cout << "VMA stats dumped to vmaStats.json" << std::endl;

        std::cerr << "Failed to create buffer: " << res << std::endl;
        throw std::runtime_error("Failed to create buffer");
    }
    // check if match VMA_ALLOCATION_CREATE_MAPPED_BIT
    if (p_buffer_type == BufferType::STAGING || p_buffer_type == BufferType::READBACK || p_buffer_type == BufferType::DYNAMIC) {
        m_mapped_memory = get_alloc_info.pMappedData;
    }
}

Buffer::Buffer() { m_ref_count.store(std::make_shared<int>(1), std::memory_order_relaxed); }

void Buffer::destruction() {
    if (m_vk_buffer != VK_NULL_HANDLE) {
        if (m_ref_count.load(std::memory_order_relaxed) && --(*m_ref_count.load()) == 0) {
            if (m_mapped_memory != nullptr) {
                vmaUnmapMemory(m_device->getAllocator(), m_allocation);
            }

            vmaDestroyBuffer(m_device->getAllocator(), m_vk_buffer, m_allocation);
        }
    }
}

Buffer::~Buffer() { destruction(); }

Buffer::Buffer(const Buffer& other)
    : m_alloc_info(other.m_alloc_info),
      m_buffer_info(other.m_buffer_info),
      m_type(other.m_type),
      m_instance_size(other.m_instance_size),
      m_instance_count(other.m_instance_count),
      m_total_size(m_instance_count * m_instance_size),
      m_allocation(other.m_allocation),
      m_vk_buffer(other.m_vk_buffer),
      m_device(other.m_device),
      m_mapped_memory(other.m_mapped_memory) {
    m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
    (*m_ref_count.load())++;
}

Buffer& Buffer::operator=(const Buffer& other) {
    if (this != &other) {
        destruction();
        m_alloc_info = other.m_alloc_info;
        m_buffer_info = other.m_buffer_info;
        m_instance_size = other.m_instance_size;
        m_instance_count = other.m_instance_count;
        m_total_size = m_instance_count * m_instance_size;
        m_allocation = other.m_allocation;
        m_vk_buffer = other.m_vk_buffer;
        m_type = other.m_type;
        m_device = other.m_device;
        m_mapped_memory = other.m_mapped_memory;
        m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
        (*m_ref_count.load())++;
    }
    return *this;
}

Buffer::Buffer(Buffer&& other)
    : m_alloc_info(other.m_alloc_info),
      m_buffer_info(other.m_buffer_info),
      m_type(other.m_type),
      m_instance_size(other.m_instance_size),
      m_instance_count(other.m_instance_count),
      m_total_size(m_instance_count * m_instance_size),
      m_allocation(other.m_allocation),
      m_vk_buffer(other.m_vk_buffer),
      m_device(other.m_device),
      m_mapped_memory(other.m_mapped_memory) {
    m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
    other.m_vk_buffer = VK_NULL_HANDLE;
    m_mapped_memory = nullptr;
}

Buffer& Buffer::operator=(Buffer&& other) {
    if (this != &other) {
        destruction();

        m_alloc_info = other.m_alloc_info;
        m_buffer_info = other.m_buffer_info;
        m_instance_size = other.m_instance_size;
        m_instance_count = other.m_instance_count;
        m_total_size = m_instance_count * m_instance_size;
        m_allocation = other.m_allocation;
        m_vk_buffer = other.m_vk_buffer;
        m_device = other.m_device;
        m_type = other.m_type;
        m_mapped_memory = other.m_mapped_memory;

        other.m_vk_buffer = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
        other.m_device = nullptr;
        m_mapped_memory = nullptr;
        m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }
    return *this;
}

VkBufferUsageFlags Buffer::getBufferUsageFlags(BufferType p_buffer_type) const {
    switch (p_buffer_type) {
        case BufferType::GPU_ONLY:
            return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        case BufferType::STAGING:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case BufferType::READBACK:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferType::DYNAMIC:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        default:
            return 0;
    }
}

VmaAllocationCreateFlags Buffer::getAllocationFlags(BufferType p_buffer_type) const {
    switch (p_buffer_type) {
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

VkDeviceAddress Buffer::getBufferDeviceAddress(uint32_t p_offset) const {
    auto buffer_device_ai = make<VkBufferDeviceAddressInfo>();
    buffer_device_ai.buffer = m_vk_buffer;
    return vkGetBufferDeviceAddress(*m_device, &buffer_device_ai) + p_offset;
}

void Buffer::writeToBuffer(void* p_data, VkDeviceSize p_size, VkDeviceSize p_offset) {
    if (p_size > 0) {
        vmaCopyMemoryToAllocation(m_device->getAllocator(), p_data, m_allocation, p_offset, p_size);
        m_mapped_memory = nullptr;
    }
}
void Buffer::readFromBuffer(void* p_data, VkDeviceSize p_size, VkDeviceSize p_offset) {
    if (p_size > 0) {
        vmaCopyAllocationToMemory(m_device->getAllocator(), m_allocation, p_offset, p_data, p_size);
        m_mapped_memory = nullptr;
    }
}

void Buffer::copyBuffer(
    Device* p_device,
    const Buffer& p_src_buffer,
    const Buffer& p_dst_buffer,
    CommandBuffer* p_ext_cmd_buffer,
    VkDeviceSize p_size,
    VkDeviceSize p_src_offset,
    VkDeviceSize p_dst_offset) {
    CommandBuffer* cmd_buffer = p_ext_cmd_buffer;
    if (cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(p_device, p_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }

    VkBufferCopy copy_region{};
    copy_region.srcOffset = p_src_offset;
    copy_region.dstOffset = p_dst_offset;

    copy_region.size = (p_size == VK_WHOLE_SIZE) ? p_src_buffer.m_total_size : p_size;
    vkCmdCopyBuffer(*cmd_buffer, p_src_buffer, p_dst_buffer, 1, &copy_region);
    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();
        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

void Buffer::addBufferMemoryBarrier(
    const CommandBuffer& p_ext_cmd_buffer, VkPipelineStageFlags2 p_src_stage_mask, VkPipelineStageFlags2 p_dst_stage_mask) {
    auto buffer_memory_barrier = make<VkBufferMemoryBarrier>();
    buffer_memory_barrier.buffer = m_vk_buffer;
    buffer_memory_barrier.srcAccessMask = getFlagFromPipelineStage(p_src_stage_mask);
    buffer_memory_barrier.dstAccessMask = getFlagFromPipelineStage(p_dst_stage_mask);
    buffer_memory_barrier.size = m_total_size;
    buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_memory_barrier.offset = 0;
    vkCmdPipelineBarrier(p_ext_cmd_buffer, p_src_stage_mask, p_dst_stage_mask, 0, 0, nullptr, 1, &buffer_memory_barrier, 0, nullptr);
}

void Buffer::transferQueueOwnership(const CommandBuffer& p_ext_cmd_buffer, uint32_t p_queue_index) {
    auto buffer_memory_barrier = make<VkBufferMemoryBarrier>();
    buffer_memory_barrier.buffer = m_vk_buffer;
    buffer_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                                        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    buffer_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    buffer_memory_barrier.size = m_total_size;
    buffer_memory_barrier.srcQueueFamilyIndex = p_ext_cmd_buffer.getQueueFamilyIndex();
    buffer_memory_barrier.dstQueueFamilyIndex = p_queue_index;
    buffer_memory_barrier.offset = 0;
    vkCmdPipelineBarrier(
        p_ext_cmd_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &buffer_memory_barrier, 0,
        nullptr);
}

void Buffer::copyToImage(Device* p_device, VkImage p_image, uint32_t p_width, uint32_t p_height, uint32_t p_layer, CommandBuffer* p_ext_cmd_buffer) {
    CommandBuffer* cmd_buffer = p_ext_cmd_buffer;
    if (cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(p_device, p_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = p_layer;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {p_width, p_height, 1};

    vkCmdCopyBufferToImage(*cmd_buffer, *this, p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();
        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

void Buffer::copyFromImage(Device* p_device, VkImage p_image, uint32_t p_width, uint32_t p_height, uint32_t p_layer, VkImageAspectFlags p_aspectMask, CommandBuffer* p_ext_cmd_buffer) {
    CommandBuffer* cmd_buffer = p_ext_cmd_buffer;
    if (cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(p_device, p_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = p_aspectMask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = p_layer;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {p_width, p_height, 1};

    vkCmdCopyImageToBuffer(*cmd_buffer, p_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *this, 1, &region);

    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();
        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

}  // namespace TTe