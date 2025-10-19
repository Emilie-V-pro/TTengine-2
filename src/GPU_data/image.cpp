
#include "GPU_data/image.hpp"



#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "commandBuffer/command_buffer.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <cstddef>

#include "GPU_data/buffer.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "stb_image.h"
#include "lodepng.h"
#include "structs_vk.hpp"
#include "utils.hpp"

#define DATA_PATH "../data/"

namespace TTe {

// define static member
VkSampler Image::s_linear_sampler = VK_NULL_HANDLE;
VkSampler Image::s_nearest_sampler = VK_NULL_HANDLE;

Image::Image(Device *p_device, ImageCreateInfo &p_image_create_info, CommandBuffer *p_ext_cmd_buffer)
    : m_image_create_info(p_image_create_info),
      m_width(p_image_create_info.width),
      m_height(p_image_create_info.height),
      m_layer(p_image_create_info.layers),
      m_image_format(p_image_create_info.format),
      m_image_layout(p_image_create_info.image_layout),
      m_device(p_device) {
    CommandBuffer *cmd = p_ext_cmd_buffer;
    if (cmd == nullptr) {
        cmd = new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(p_device, p_device->getRenderQueue())->createCommandBuffer(1)[0]));
        cmd->beginCommandBuffer();
    }

    m_ref_count.store(std::make_shared<int>(1), std::memory_order_relaxed);

    if (!this->m_image_create_info.filename.empty()) {
        loadImageFromFile(m_image_create_info.filename);
    }

    createImage();
    createImageView();

    if (this->m_image_create_info.datas.size() > 0) {
        loadImageToGPU(cmd);

        if (p_image_create_info.enable_mipmap) {
            generateMipmaps(cmd);
        }

    } else {
        transitionImageLayout(m_image_layout, cmd);
    }
    if (p_ext_cmd_buffer == nullptr) {
        cmd->endCommandBuffer();
        cmd->addRessourceToDestroy(cmd);
        cmd->submitCommandBuffer({}, {}, nullptr, false);
    }
}

Image::Image(Device *p_device, VkImage p_image, VkImageView p_image_view, VkFormat p_format, VkExtent2D p_swap_chain_extent)
    : m_width(p_swap_chain_extent.width),
      m_height(p_swap_chain_extent.height),
      m_layer(1),
      m_image_format(p_format),
      m_image_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
      m_actual_image_layout(VK_IMAGE_LAYOUT_UNDEFINED),
      m_device(p_device),
      m_vk_image(p_image),
      m_image_view(p_image_view),
      m_is_swapchain_image(true) {}

Image::Image(const Image &other)
    : name(other.name),
      m_image_create_info(other.m_image_create_info),
      m_width(other.m_width),
      m_height(other.m_height),
      m_layer(other.m_layer),
      m_mip_levels(other.m_mip_levels),
      m_image_format(other.m_image_format),
      m_image_layout(other.m_image_layout),
      m_actual_image_layout(other.m_actual_image_layout),
      m_device(other.m_device),
      m_vk_image(other.m_vk_image),
      m_image_view(other.m_image_view),
      m_allocation(other.m_allocation),
      m_is_swapchain_image(other.m_is_swapchain_image) {
    if (!other.m_is_swapchain_image) {
        m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
        (*m_ref_count.load())++;
    }
}

Image::Image(Image &&other)
    : name(other.name),
      m_image_create_info(other.m_image_create_info),
      m_width(other.m_width),
      m_height(other.m_height),
      m_layer(other.m_layer),
      m_mip_levels(other.m_mip_levels),
      m_image_format(other.m_image_format),
      m_image_layout(other.m_image_layout),
      m_actual_image_layout(other.m_actual_image_layout),
      m_device(other.m_device),
      m_vk_image(other.m_vk_image),
      m_image_view(other.m_image_view),
      m_allocation(other.m_allocation),
      m_is_swapchain_image(other.m_is_swapchain_image)
    {
    if (!other.m_is_swapchain_image) {
        m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
        other.m_vk_image = VK_NULL_HANDLE;
    }
}

void Image::destruction() {
    if (!m_is_swapchain_image && m_vk_image != VK_NULL_HANDLE) {
        if (m_ref_count.load(std::memory_order_relaxed) && --(*m_ref_count.load()) == 0) {
            if (m_vk_image != VK_NULL_HANDLE) {
                vmaDestroyImage(m_device->getAllocator(), m_vk_image, m_allocation);
                // vmaFreeMemory(device->getAllocator(), m_allocation);
            }
            if (m_image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(*m_device, m_image_view, nullptr);
            }
        }
    }
}

Image::~Image() { destruction(); }

Image &Image::operator=(const Image &other) {
    if (this != &other) {
        destruction();
        m_image_create_info = other.m_image_create_info;
        m_width = other.m_width;
        m_height = other.m_height;
        m_layer = other.m_layer;
        m_mip_levels = other.m_mip_levels;
        m_device = other.m_device;
        m_vk_image = other.m_vk_image;
        m_allocation = other.m_allocation;
        m_image_view = other.m_image_view;
        m_image_format = other.m_image_format;
        m_image_layout = other.m_image_layout;
        m_actual_image_layout = other.m_actual_image_layout;
        m_is_swapchain_image = other.m_is_swapchain_image;
        name = other.name;
        if (!other.m_is_swapchain_image) {
            m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
            (*m_ref_count.load())++;
        }
    }
    return *this;
}

Image &Image::operator=(Image &&other) {
    if (this != &other) {
        destruction();
        m_image_create_info = other.m_image_create_info;
        m_width = other.m_width;
        m_height = other.m_height;
        m_layer = other.m_layer;
        m_mip_levels = other.m_mip_levels;
        m_device = other.m_device;
        m_vk_image = other.m_vk_image;
        m_allocation = other.m_allocation;
        m_image_view = other.m_image_view;
        m_image_format = other.m_image_format;
        m_image_layout = other.m_image_layout;
        m_actual_image_layout = other.m_actual_image_layout;
        m_is_swapchain_image = other.m_is_swapchain_image;
        name = other.name;
        if (!m_is_swapchain_image) {
            other.m_vk_image = VK_NULL_HANDLE;
            m_ref_count.store(other.m_ref_count.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
    }
    return *this;
}

void Image::writeToImage(void *p_data, size_t p_size, uint32_t p_offset, CommandBuffer *p_ext_cmd_buffer) {
    CommandBuffer *cmd_buffer = p_ext_cmd_buffer;
    if (cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }

    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buffer);
    Buffer *b = new Buffer(m_device, getPixelSizeFromFormat(m_image_format), static_cast<u_int32_t>(p_size), 0, Buffer::BufferType::STAGING, 0);
    b->writeToBuffer(p_data, p_size, p_offset);
    b->copyToImage(m_device, *this, m_width, m_height, m_layer, cmd_buffer);
    transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buffer);
    cmd_buffer->addRessourceToDestroy(b);

    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();
        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, true);
    }
}

uint16_t toUint16(float p_depth) {
    // Clamp entre 0 et 1, puis mise à l’échelle
    p_depth = std::min(std::max(p_depth, 0.0f), 1.0f);
    return static_cast<uint16_t>(p_depth * 65535.0f + 0.5f);
}



unsigned encodeDepthPNG(const char *p_filename, const std::vector<unsigned char> &p_image, unsigned p_width, unsigned p_height) {
    LodePNGColorType color_type = LCT_GREY;
    unsigned bitdepth = 16;

    unsigned error = lodepng::encode(p_filename, p_image.data(), p_width, p_height, color_type, bitdepth);
    return error;
}

void Image::saveImageToFile() {
    CommandBuffer *cmd_buffer =

        new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getRenderQueue())->createCommandBuffer(1)[0]));
    cmd_buffer->beginCommandBuffer();

    Buffer *b = new Buffer(
        m_device, getPixelSizeFromFormat(m_image_format), static_cast<u_int32_t>(m_width * m_height * m_layer), 0, Buffer::BufferType::READBACK, 0);

    VkImageLayout oldLayout = m_actual_image_layout;
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd_buffer);
    b->copyFromImage(m_device, *this, m_width, m_height, 1, (m_image_format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,cmd_buffer);
    transitionImageLayout(oldLayout, cmd_buffer);

    cmd_buffer->endCommandBuffer();
    cmd_buffer->submitCommandBuffer({}, {}, nullptr, true);

    delete cmd_buffer;

    switch (m_image_format) {
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM: {
            std::vector<unsigned char> image_data(m_width * m_height * 4);

            b->readFromBuffer(image_data.data(), m_width * m_height * m_layer * 4);

            unsigned error = lodepng::encode(name + ".png", image_data, m_width, m_height, LodePNGColorType::LCT_RGBA);
            if (error) {
                std::cout << "Error encoding PNG: " << lodepng_error_text(error) << std::endl;
            }
            break;
        }
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT: {
            std::vector<float> image_data(m_width * m_height);
            b->readFromBuffer(image_data.data(), m_width * m_height * m_layer * 4);
            std::vector<unsigned char> depth_data(m_width * m_height * 2);

            for (size_t i = 0; i < m_width * m_height; i++) {
                uint16_t v = toUint16(image_data[i]);
                depth_data[2 * i + 0] = (v >> 8) & 0xFF;  // MSB
                depth_data[2 * i + 1] = v & 0xFF;         // LSB
            }
            unsigned error = encodeDepthPNG((name + ".png").c_str(), depth_data, m_width, m_height);
            if (error) {
                std::cout << "Error encoding PNG: " << lodepng_error_text(error) << std::endl;
            }
            break;
        }
        default:
            std::cerr << "Format not supported for saving image" << std::endl;
            break;
    }

    delete b;
}

void Image::transitionImageLayout(VkImageLayout p_new_layout, CommandBuffer *p_ext_cmd_buffer) {
    transitionImageLayout(m_actual_image_layout, p_new_layout, 0, this->m_mip_levels, p_ext_cmd_buffer);
    m_actual_image_layout = p_new_layout;
}

void Image::transitionImageLayout(
    VkImageLayout p_old_layout, VkImageLayout p_new_layout, u_int32_t p_mip_level, u_int32_t p_mip_count, CommandBuffer *p_ext_cmd_buffer) {
    if (p_old_layout == p_new_layout) {
        return;
    }
    auto barrier = make<VkImageMemoryBarrier>();
    barrier.oldLayout = p_old_layout;                          // VK_IMAGE_LAYOUT_UNDEFINED
    barrier.newLayout = p_new_layout;                          // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // VK_QUEUE_FAMILY_IGNORED
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // VK_QUEUE_FAMILY_IGNORED
    barrier.image = m_vk_image;
    if (m_image_format != VK_FORMAT_D32_SFLOAT) {  // https://discord.com/channels/231931740661350410/1140282527760781323
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // VK_IMAGE_ASPECT_COLOR_BIT
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    barrier.subresourceRange.baseMipLevel = p_mip_level;
    barrier.subresourceRange.levelCount = p_mip_count;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_layer;

    barrier.srcAccessMask = getAccessFlagsFromLayout(p_old_layout);
    barrier.dstAccessMask = getAccessFlagsFromLayout(p_new_layout);

    // TODO : GET CORRECT STATE FLAGS
    VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    CommandBuffer *cmd_buffer = p_ext_cmd_buffer;
    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }

    vkCmdPipelineBarrier(*cmd_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();
        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

void Image::addImageMemoryBarrier(
    const CommandBuffer &p_ext_cmd_buffer, VkPipelineStageFlags2 p_src_stage_mask, VkPipelineStageFlags2 p_dst_stage_mask) {
    auto image_memory_barrier = make<VkImageMemoryBarrier>();
    image_memory_barrier.oldLayout = m_actual_image_layout;
    image_memory_barrier.newLayout = m_actual_image_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = m_vk_image;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = m_mip_levels;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = m_layer;
    image_memory_barrier.srcAccessMask = getFlagFromPipelineStage(p_src_stage_mask);
    image_memory_barrier.dstAccessMask = getFlagFromPipelineStage(p_dst_stage_mask);
    vkCmdPipelineBarrier(p_ext_cmd_buffer, p_src_stage_mask, p_dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void Image::transferQueueOwnership(const CommandBuffer &p_ext_cmd_buffer, uint32_t p_queue_index) {
    auto image_memory_barrier = make<VkImageMemoryBarrier>();
    image_memory_barrier.oldLayout = m_actual_image_layout;
    image_memory_barrier.newLayout = m_actual_image_layout;
    image_memory_barrier.srcQueueFamilyIndex = p_ext_cmd_buffer.getQueueFamilyIndex();
    image_memory_barrier.dstQueueFamilyIndex = p_queue_index;
    image_memory_barrier.image = m_vk_image;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = m_mip_levels;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = m_layer;
    image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                                       VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    vkCmdPipelineBarrier(
        p_ext_cmd_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
        &image_memory_barrier);
}

void Image::generateMipmaps(CommandBuffer *p_ext_cmd_buffer) {
    CommandBuffer *cmd;
    if (p_ext_cmd_buffer == nullptr) {
        cmd = new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getRenderQueue())->createCommandBuffer(1)[0]));
        cmd->beginCommandBuffer();
    } else {
        cmd = p_ext_cmd_buffer;
    }
    transitionImageLayout(m_actual_image_layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1, cmd);

    for (uint32_t i = 1; i < m_mip_levels; i++) {
        VkImageBlit image_blit{};

        // Source
        image_blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_blit.srcSubresource.layerCount = m_layer;
        image_blit.srcSubresource.mipLevel = i - 1;
        image_blit.srcOffsets[1].x = int32_t(this->m_width >> (i - 1));
        image_blit.srcOffsets[1].y = int32_t(this->m_height >> (i - 1));
        image_blit.srcOffsets[1].z = 1;

        // Destination
        image_blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_blit.dstSubresource.layerCount = m_layer;
        image_blit.dstSubresource.mipLevel = i;
        image_blit.dstOffsets[1].x = int32_t(this->m_width >> i);
        image_blit.dstOffsets[1].y = int32_t(this->m_height >> i);
        image_blit.dstOffsets[1].z = 1;

        VkImageSubresourceRange mip_sub_range = {};
        mip_sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        mip_sub_range.baseMipLevel = i;
        mip_sub_range.levelCount = 1;
        mip_sub_range.layerCount = m_layer;

        // Prepare current mip level as image blit destination
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i, 1, cmd);

        // Blit from previous level
        vkCmdBlitImage(
            *cmd, this->m_vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->m_vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_blit,
            VK_FILTER_LINEAR);

        // // Prepare current mip level as image blit source for next level
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i, 1, cmd);
    }
    m_actual_image_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    // transitioto shader read only optimal
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd);

    if (p_ext_cmd_buffer == nullptr) {
        cmd->endCommandBuffer();
        cmd->addRessourceToDestroy(cmd);
        cmd->submitCommandBuffer({}, {}, nullptr, true);
    }
}

void Image::createImage() {
    if (m_image_create_info.enable_mipmap) {
        m_image_create_info.usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        m_mip_levels = std::min(
            static_cast<uint32_t>(std::floor(std::log2(std::max(m_image_create_info.width, m_image_create_info.width)))) + 1, uint32_t(16));
    }
    auto image_info = make<VkImageCreateInfo>();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = m_image_create_info.format;
    image_info.extent.width = m_image_create_info.width;
    image_info.extent.height = m_image_create_info.height;

    image_info.mipLevels = m_mip_levels;
    image_info.arrayLayers = m_image_create_info.layers;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = m_image_create_info.usage_flags;
    if (m_image_create_info.datas.size() > 0) {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    image_info.extent = {m_image_create_info.width, m_image_create_info.height, 1};
    // imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    if (m_image_create_info.is_cube_texture) {
        image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_actual_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void Image::createImageWithInfo(const VkImageCreateInfo &p_image_info, VkMemoryPropertyFlags p_properties) {
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = p_properties;

    auto res = vmaCreateImage(m_device->getAllocator(), &p_image_info, &alloc_info, &m_vk_image, &m_allocation, nullptr);
    if (res != VK_SUCCESS) {
        // dump vma
        std::ofstream file("vmaStats.json");
        char *s;
        vmaBuildStatsString(m_device->getAllocator(), &s, VK_TRUE);

        file << s;
        file.close();
        vmaFreeStatsString(m_device->getAllocator(), s);
        std::cout << "VMA stats dumped to vmaStats.json" << std::endl;

        std::cerr << "Failed to create buffer: " << res << std::endl;
        throw std::runtime_error("Failed to create buffer");
    }
}

void Image::createImageView() {
    auto image_view_info = make<VkImageViewCreateInfo>();
    if (m_image_create_info.is_cube_texture) {
        if ((m_image_create_info.layers / 6) > 1) {
            image_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        } else {
            image_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }
    } else {
        if (m_image_create_info.layers > 1) {
            image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        } else {
            image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;  // VK_IMAGE_VIEW_TYPE_2D
        }
    }

    image_view_info.format = m_image_format;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // VK_IMAGE_ASPECT_COLOR_BIT
    if (m_image_create_info.usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = m_image_create_info.layers;
    image_view_info.subresourceRange.levelCount = m_mip_levels;
    image_view_info.image = m_vk_image;
    image_view_info.flags = 0;
    auto result = vkCreateImageView(*m_device, &image_view_info, nullptr, &m_image_view);
    if (result != VK_SUCCESS) {
        std::cerr << "Erreur: vkCreateImageView a échoué avec le code " << result << std::endl;
    }
}

void Image::loadImageFromFile(std::vector<std::filesystem::path> &p_filename) {
    stbi_set_flip_vertically_on_load(true);
    int nb_of_channel;
    int width = 0, height = 0;
    std::cout << "Loading image: " <<DATA_PATH / p_filename[0] << std::endl;
    stbi_info((DATA_PATH/p_filename[0]).c_str(), &width, &height, &nb_of_channel);
    if (width == 0 || height == 0) {
        std::cerr << "Erreur: l'image n'a pas pu être chargée" << std::endl;
    }
    this->m_width = width;
    this->m_height = height;
    m_image_create_info.width = width;
    m_image_create_info.height = height;
    m_image_create_info.layers = p_filename.size();
    this->m_layer = p_filename.size();
    for (size_t i = 0; i < p_filename.size(); i++) {
        m_image_create_info.datas.push_back(stbi_load((DATA_PATH /p_filename[i]).c_str(), &width, &height, &nb_of_channel, 4));
    }

    if (m_image_create_info.format == VK_FORMAT_UNDEFINED) {
        this->m_image_format = VK_FORMAT_R8G8B8A8_SRGB;
        m_image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    }
}

void Image::loadImageToGPU(CommandBuffer *p_ext_cmd_buffer) {
    CommandBuffer *cmd_buffer = p_ext_cmd_buffer;
    if (cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }

    VkDeviceSize size = m_width * m_height * getPixelSizeFromFormat(m_image_format);
    Buffer *b = new Buffer(
        m_device, getPixelSizeFromFormat(m_image_format), static_cast<u_int32_t>(m_width * m_height * m_layer), 0, Buffer::BufferType::STAGING, 0);
    for (size_t i = 0; i < m_image_create_info.datas.size(); i++) {
        b->writeToBuffer(m_image_create_info.datas[i], size, i * size);
    }
    if (m_image_create_info.filename.size() > 0) {
        for (size_t i = 0; i < m_image_create_info.datas.size(); i++) {
            stbi_image_free(m_image_create_info.datas[i]);
        }
    }
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buffer);
    b->copyToImage(m_device, *this, m_width, m_height, m_layer, cmd_buffer);
    transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd_buffer);
    cmd_buffer->addRessourceToDestroy(b);

    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();

        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, true);
    }
}

void Image::blitImage(Device *p_device, Image &p_src_image, Image &p_dst_image, CommandBuffer *p_ext_cmd_buffer) {
    CommandBuffer *p_cmd_buffer = p_ext_cmd_buffer;
    if (p_cmd_buffer == nullptr) {
        p_cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(p_device, p_device->getTransferQueue())->createCommandBuffer(1)[0]));
        p_cmd_buffer->beginCommandBuffer();
    }

    p_src_image.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, p_cmd_buffer);
    p_dst_image.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, p_cmd_buffer);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {(int32_t)p_src_image.getWidth(), (int32_t)p_src_image.getHeight(), 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = 0;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = p_src_image.m_layer;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {(int32_t)p_src_image.getWidth(), (int32_t)p_src_image.getHeight(), 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = 0;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = p_src_image.m_layer;

    vkCmdBlitImage(
        *p_cmd_buffer, p_src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, p_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
        VK_FILTER_LINEAR);
}

void Image::copyImage(Device *p_device, Image &p_src_image, Image &p_dst_image, CommandBuffer *p_ext_cmd_buffer) {
    CommandBuffer *cmd_buffer = p_ext_cmd_buffer;
    if (cmd_buffer == nullptr) {
        cmd_buffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(p_device, p_device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmd_buffer->beginCommandBuffer();
    }

    p_src_image.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd_buffer);
    p_dst_image.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buffer);

    VkImageCopy copy_region{};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.layerCount = p_src_image.m_layer;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.layerCount = p_src_image.m_layer;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.extent.width = p_src_image.m_width;
    copy_region.extent.height = p_src_image.m_height;
    copy_region.extent.depth = 1;

    vkCmdCopyImage(
        *cmd_buffer, p_src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, p_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    if (p_ext_cmd_buffer == nullptr) {
        cmd_buffer->endCommandBuffer();
        cmd_buffer->addRessourceToDestroy(cmd_buffer);
        cmd_buffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

void Image::createsamplers(Device *p_device) {
    auto sampler_info = make<VkSamplerCreateInfo>();
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 8;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = VK_LOD_CLAMP_NONE;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    auto result = vkCreateSampler(*p_device, &sampler_info, nullptr, &s_linear_sampler);
    if (result != VK_SUCCESS) {
        std::cerr << "Erreur: vkCreateSampler a échoué avec le code " << result << std::endl;
    }

    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    result = vkCreateSampler(*p_device, &sampler_info, nullptr, &s_nearest_sampler);
}

void Image::destroySamplers(Device *p_device) {
    vkDestroySampler(*p_device, s_linear_sampler, nullptr);
    vkDestroySampler(*p_device, s_nearest_sampler, nullptr);
}

}  // namespace TTe