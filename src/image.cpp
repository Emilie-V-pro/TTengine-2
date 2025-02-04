
#include "image.hpp"

#include <vulkan/vulkan_core.h>

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <cstddef>

#include "buffer.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "stb_image.h"
#include "structs_vk.hpp"
#include "utils.hpp"

namespace TTe {

Image::Image(Device *device, ImageCreateInfo &imageCreateInfo, CommandBuffer *cmdBuffer)
    : imageCreateInfo(imageCreateInfo),
      width(imageCreateInfo.width),
      height(imageCreateInfo.height),
      layer(imageCreateInfo.layers),
      imageFormat(imageCreateInfo.format),
      imageLayout(imageCreateInfo.imageLayout),
      device(device) {
    std::cout << "Image Constructor" << std::endl;
    mutex.lock();
    refCount = new int(1);
    if (!this->imageCreateInfo.filename.empty()) {
        loadImageFromFile(imageCreateInfo.filename);
    }
    createImage();
    createImageView();

    if (this->imageCreateInfo.datas.size() > 0) {
        loadImageToGPU(cmdBuffer);
    }

    mutex.unlock();
}

Image::Image(Device *device, VkImage image, VkImageView imageView, VkFormat format, VkExtent2D swapChainExtent)
    : width(swapChainExtent.width),
      height(swapChainExtent.height),
      layer(1),
      imageFormat(format),
      imageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
      actualImageLayout(VK_IMAGE_LAYOUT_UNDEFINED),
      device(device),
      vk_image(image),
      imageView(imageView),
      isSwapchainImage(true) {}

Image::Image(Image &other)
    : imageCreateInfo(other.imageCreateInfo),
      width(other.width),
      height(other.height),
      layer(other.layer),
      mipLevels(other.mipLevels),
      imageFormat(other.imageFormat),
      imageLayout(other.imageLayout),
      actualImageLayout(other.actualImageLayout),
      device(other.device),
      vk_image(other.vk_image),
      imageView(other.imageView),
      allocation(other.allocation),
      imageMemory(other.imageMemory),
      isSwapchainImage(other.isSwapchainImage) {
    std::cout << "Image Copy Constructor" << std::endl;
    mutex.lock();
    other.mutex.lock();
    refCount = other.refCount;
    *refCount += 1;
    other.mutex.unlock();
    mutex.unlock();
}

Image::Image(Image &&other)
    : imageCreateInfo(other.imageCreateInfo),
      width(other.width),
      height(other.height),
      layer(other.layer),
      mipLevels(other.mipLevels),
      imageFormat(other.imageFormat),
      imageLayout(other.imageLayout),
      actualImageLayout(other.actualImageLayout),
      device(other.device),
      vk_image(other.vk_image),
      imageView(other.imageView),
      allocation(other.allocation),
      imageMemory(other.imageMemory),
      isSwapchainImage(other.isSwapchainImage) {
    std::cout << "Image Move Constructor" << std::endl;
    mutex.lock();
    other.mutex.lock();
    refCount = other.refCount;
    other.refCount = nullptr;
    other.vk_image = VK_NULL_HANDLE;
    other.mutex.unlock();
    mutex.unlock();

}

Image::~Image() {
    std::cout << "Image Destructor" << std::endl;
    mutex.lock();
    if (!isSwapchainImage && vk_image != VK_NULL_HANDLE) {
        std::cout << "refCount : " << *refCount << std::endl;
        *refCount -= 1;
        std::cout << "refCount : " << *refCount << std::endl;
        if (*refCount == 0) {
            delete refCount;

            if (vk_image != VK_NULL_HANDLE) {
                vmaDestroyImage(device->getAllocator(), vk_image, allocation);
                // vmaFreeMemory(device->getAllocator(), allocation);
            }
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(*device, imageView, nullptr);
            }
        }
    }
    mutex.unlock();
}

Image &Image::operator=(Image &other) {
    if (this != &other) {
        std::cout << "Image Copy Operator" << std::endl;
        other.mutex.lock();

        this->~Image();
        mutex.lock();
        imageCreateInfo = other.imageCreateInfo;
        width = other.width;
        height = other.height;
        layer = other.layer;
        mipLevels = other.mipLevels;
        device = other.device;
        vk_image = other.vk_image;
        allocation = other.allocation;
        imageMemory = other.imageMemory;
        imageView = other.imageView;
        imageFormat = other.imageFormat;
        imageLayout = other.imageLayout;
        actualImageLayout = other.actualImageLayout;
        isSwapchainImage = other.isSwapchainImage;

        refCount = other.refCount;
        *refCount += 1;
        other.mutex.unlock();
        mutex.unlock();
    }
    return *this;
}

Image &Image::operator=(Image &&other) {
    if (this != &other) {
        std::cout << "Image Move Operator" << std::endl;
        other.mutex.lock();

        this->~Image();
        mutex.lock();
        imageCreateInfo = other.imageCreateInfo;
        width = other.width;
        height = other.height;
        layer = other.layer;
        mipLevels = other.mipLevels;
        device = other.device;
        vk_image = other.vk_image;
        allocation = other.allocation;
        imageMemory = other.imageMemory;
        imageView = other.imageView;
        imageFormat = other.imageFormat;
        imageLayout = other.imageLayout;
        actualImageLayout = other.actualImageLayout;
        isSwapchainImage = other.isSwapchainImage;
        other.vk_image = VK_NULL_HANDLE;

        refCount = other.refCount;
        other.refCount = nullptr;
        other.mutex.unlock();
        mutex.unlock();
    }
    return *this;
}

void Image::transitionImageLayout(VkImageLayout newLayout, CommandBuffer *extCmdBuffer) {
    transitionImageLayout(actualImageLayout, newLayout, 0, this->mipLevels, extCmdBuffer);
    actualImageLayout = newLayout;
}

void Image::transitionImageLayout(
    VkImageLayout oldLayout, VkImageLayout newLayout, u_int32_t mipLevel, u_int32_t mipCount, CommandBuffer *extCmdBuffer) {
    if (oldLayout == newLayout) {
        return;
    }
    auto barrier = make<VkImageMemoryBarrier>();
    barrier.oldLayout = oldLayout;                          // VK_IMAGE_LAYOUT_UNDEFINED
    barrier.newLayout = newLayout;                          // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // VK_QUEUE_FAMILY_IGNORED
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // VK_QUEUE_FAMILY_IGNORED
    barrier.image = vk_image;
    if (imageFormat != VK_FORMAT_D32_SFLOAT) {  // https://discord.com/channels/231931740661350410/1140282527760781323
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // VK_IMAGE_ASPECT_COLOR_BIT
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    barrier.subresourceRange.baseMipLevel = mipLevel;
    barrier.subresourceRange.levelCount = mipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layer;

    barrier.srcAccessMask = getAccessFlagsFromLayout(oldLayout);
    barrier.dstAccessMask = getAccessFlagsFromLayout(newLayout);

    // TODO : GET CORRECT STATE FLAGS
    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    CommandBuffer *cmdBuffer = extCmdBuffer;
    if (extCmdBuffer == nullptr) {
        cmdBuffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmdBuffer->beginCommandBuffer();
    }

    vkCmdPipelineBarrier(*cmdBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    if (extCmdBuffer == nullptr) {
        cmdBuffer->endCommandBuffer();
        cmdBuffer->addRessourceToDestroy(cmdBuffer);
        cmdBuffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

void Image::addImageMemoryBarrier(
    const CommandBuffer &extCmdBuffer, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask) {
    auto imageMemoryBarrier = make<VkImageMemoryBarrier>();
    imageMemoryBarrier.oldLayout = actualImageLayout;
    imageMemoryBarrier.newLayout = actualImageLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = vk_image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = layer;
    imageMemoryBarrier.srcAccessMask = getFlagFromPipelineStage(srcStageMask);
    imageMemoryBarrier.dstAccessMask = getFlagFromPipelineStage(dstStageMask);
    vkCmdPipelineBarrier(extCmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void Image::transferQueueOwnership(const CommandBuffer &extCmdBuffer, uint32_t queueIndex) {
    auto imageMemoryBarrier = make<VkImageMemoryBarrier>();
    imageMemoryBarrier.oldLayout = actualImageLayout;
    imageMemoryBarrier.newLayout = actualImageLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = extCmdBuffer.getQueueFamilyIndex();
    imageMemoryBarrier.dstQueueFamilyIndex = queueIndex;
    imageMemoryBarrier.image = vk_image;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = layer;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                                       VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    vkCmdPipelineBarrier(
        extCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
        &imageMemoryBarrier);
}

void Image::generateMipmaps() {}

void Image::createImage() {
    if (imageCreateInfo.enableMipMap) {
        imageCreateInfo.usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageCreateInfo.width, imageCreateInfo.width)))) + 1;
    }
    auto imageInfo = make<VkImageCreateInfo>();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = imageCreateInfo.format;
    imageInfo.extent.width = imageCreateInfo.width;
    imageInfo.extent.height = imageCreateInfo.height;

    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = imageCreateInfo.layers;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imageCreateInfo.usageFlags;
    if (imageCreateInfo.datas.size() > 0) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    imageInfo.extent = {imageCreateInfo.width, imageCreateInfo.height, 1};
    // imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    if (imageCreateInfo.isCubeTexture) {
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    actualImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void Image::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties) {
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = properties;
    // std::cout << imageInfo.extent.width << " " << imageInfo.extent.height << std::endl;

    vmaCreateImage(device->getAllocator(), &imageInfo, &allocInfo, &vk_image, &allocation, nullptr);
 
}

void Image::createImageView() {
    auto imageViewInfo = make<VkImageViewCreateInfo>();
    if (imageCreateInfo.isCubeTexture) {
        if ((imageCreateInfo.layers / 6) > 1) {
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        } else {
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }
    } else {
        if (imageCreateInfo.layers > 1) {
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        } else {
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // VK_IMAGE_VIEW_TYPE_2D
        }
    }

    imageViewInfo.format = imageFormat;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // VK_IMAGE_ASPECT_COLOR_BIT
    if (imageCreateInfo.usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = imageCreateInfo.layers;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.image = vk_image;
    imageViewInfo.flags = 0;
    auto result = vkCreateImageView(*device, &imageViewInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
        std::cerr << "Erreur: vkCreateImageView a échoué avec le code " << result << std::endl;
    }
}

#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>

int test() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }
    return 0;
}

void Image::loadImageFromFile(std::vector<std::string> &filename) {
    // stbi_set_flip_vertically_on_load(true);
    int nbOfchannel;
    int width, height;
    test();
    stbi_info(filename[0].c_str(), &width, &height, &nbOfchannel);
    this->width = width;
    this->height = height;
    imageCreateInfo.width = width;
    imageCreateInfo.height = height;
    std::cout << width << " " << height << std::endl;
    for (size_t i = 0; i < filename.size(); i++) {
        std::cout << filename[i] << std::endl;
        imageCreateInfo.datas.push_back(stbi_load((filename[i]).c_str(), &width, &height, &nbOfchannel, 4));
    }

    if (imageCreateInfo.format == VK_FORMAT_UNDEFINED) {
        this->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    }
}

void Image::loadImageToGPU(CommandBuffer *extCmdBuffer) {
    CommandBuffer *cmdBuffer = extCmdBuffer;
    if (cmdBuffer == nullptr) {
        cmdBuffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmdBuffer->beginCommandBuffer();
    }
    VkDeviceSize size = width * height * getPixelSizeFromFormat(imageFormat);
    Buffer *b = new Buffer(
        device, getPixelSizeFromFormat(imageFormat), static_cast<u_int32_t>(width * height * layer), 0, Buffer::BufferType::STAGING);
    for (size_t i = 0; i < imageCreateInfo.datas.size(); i++) {
        b->writeToBuffer(imageCreateInfo.datas[i], size, i * size);
    }
    if (imageCreateInfo.filename.size() > 0) {
        for (size_t i = 0; i < imageCreateInfo.datas.size(); i++) {
            stbi_image_free(imageCreateInfo.datas[i]);
        }
    }
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);
    b->copyToImage(device, *this, width, height, layer, cmdBuffer);
    transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdBuffer);

    if (extCmdBuffer == nullptr) {
        cmdBuffer->endCommandBuffer();
        cmdBuffer->addRessourceToDestroy(cmdBuffer);
        cmdBuffer->addRessourceToDestroy(b);
        cmdBuffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

void Image::blitImage(Device *device, Image &srcImage, Image &dstImage, CommandBuffer *extCmdBuffer) {
    CommandBuffer *cmdBuffer = extCmdBuffer;
    if (cmdBuffer == nullptr) {
        cmdBuffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmdBuffer->beginCommandBuffer();
    }

    srcImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmdBuffer);
    dstImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {(int32_t)srcImage.getWidth(), (int32_t)srcImage.getHeight(), 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = 0;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = srcImage.layer;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {(int32_t)srcImage.getWidth(), (int32_t)srcImage.getHeight(), 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = 0;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = srcImage.layer;

    vkCmdBlitImage(
        *cmdBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
        VK_FILTER_LINEAR);
}

void Image::copyImage(Device *device, Image &srcImage, Image &dstImage, CommandBuffer *extCmdBuffer) {
    CommandBuffer *cmdBuffer = extCmdBuffer;
    if (cmdBuffer == nullptr) {
        cmdBuffer =
            new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
        cmdBuffer->beginCommandBuffer();
    }

    srcImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmdBuffer);
    dstImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);

    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = srcImage.layer;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.layerCount = srcImage.layer;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.extent.width = srcImage.width;
    copyRegion.extent.height = srcImage.height;
    copyRegion.extent.depth = 1;

    vkCmdCopyImage(
        *cmdBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    if (extCmdBuffer == nullptr) {
        cmdBuffer->endCommandBuffer();
        cmdBuffer->addRessourceToDestroy(cmdBuffer);
        cmdBuffer->submitCommandBuffer({}, {}, nullptr, false);
    }
}

}  // namespace TTe