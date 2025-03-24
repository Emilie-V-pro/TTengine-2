
#include "GPU_data/image.hpp"

#include <iostream>
#include "commandBuffer/command_buffer.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <cstddef>

#include "GPU_data/buffer.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "stb_image.h"
#include "structs_vk.hpp"
#include "utils.hpp"

#define TEXTURE_PATH "../data/textures/"

namespace TTe {

// define static member
VkSampler Image::linearSampler = VK_NULL_HANDLE;
VkSampler Image::nearestSampler = VK_NULL_HANDLE;

Image::Image(Device *device, ImageCreateInfo &imageCreateInfo, CommandBuffer *extcmdBuffer)
    : imageCreateInfo(imageCreateInfo),
      width(imageCreateInfo.width),
      height(imageCreateInfo.height),
      layer(imageCreateInfo.layers),
      imageFormat(imageCreateInfo.format),
      imageLayout(imageCreateInfo.imageLayout),
      device(device) {
    CommandBuffer *cmd = extcmdBuffer;
    if (cmd == nullptr) {
        cmd  = new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getRenderQueue())->createCommandBuffer(1)[0]));
        cmd->beginCommandBuffer();
    }
    
        refCount.store(std::make_shared<int>(1), std::memory_order_relaxed);
    if (!this->imageCreateInfo.filename.empty()) {
        loadImageFromFile(imageCreateInfo.filename);
    }
    createImage();
    createImageView();

    if (this->imageCreateInfo.datas.size() > 0) {
        loadImageToGPU(cmd);
        if(imageCreateInfo.enableMipMap){
            generateMipmaps(cmd);
        }
    }else{
        transitionImageLayout(imageLayout, cmd);
    }
    if (extcmdBuffer == nullptr) {
        cmd->endCommandBuffer();
        cmd->addRessourceToDestroy(cmd);
        cmd->submitCommandBuffer({}, {}, nullptr, true);
   
    }
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

Image::Image(const Image &other)
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
      isSwapchainImage(other.isSwapchainImage),
      name(other.name) {
    if (!other.isSwapchainImage) {
        refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
        (*refCount.load())++;
    }
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
      isSwapchainImage(other.isSwapchainImage),
      name(other.name) {
    if (!other.isSwapchainImage) {
        refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
        other.vk_image = VK_NULL_HANDLE;
    }
}

void Image::destruction() {
    if (!isSwapchainImage && vk_image != VK_NULL_HANDLE) {
        if (refCount.load(std::memory_order_relaxed) && --(*refCount.load()) == 0) {
            if (vk_image != VK_NULL_HANDLE) {
                vmaDestroyImage(device->getAllocator(), vk_image, allocation);
                // vmaFreeMemory(device->getAllocator(), allocation);
            }
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(*device, imageView, nullptr);
            }
        }
    }
}

Image::~Image() { destruction(); }

Image &Image::operator=(const Image &other) {
    if (this != &other) {
        destruction();
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
        name = other.name;
        if (!other.isSwapchainImage) {
            refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
            (*refCount.load())++;
        }
    }
    return *this;
}

Image &Image::operator=(Image &&other) {
    if (this != &other) {

        destruction();
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
        name = other.name;
        if (!isSwapchainImage) {
            other.vk_image = VK_NULL_HANDLE;
            refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
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

void Image::generateMipmaps(CommandBuffer *extCmdBuffer) {
    CommandBuffer* cmd;
    if (extCmdBuffer == nullptr) {
        cmd = new CommandBuffer(std::move(CommandPoolHandler::getCommandPool(device, device->getRenderQueue())->createCommandBuffer(1)[0]));
        cmd->beginCommandBuffer();
    } else {
        cmd = extCmdBuffer;
    }
    transitionImageLayout(actualImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1, cmd);


    for (uint32_t i = 1; i < mipLevels; i++) {
        VkImageBlit imageBlit{};

        // Source
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = layer;
        imageBlit.srcSubresource.mipLevel = i - 1;
        imageBlit.srcOffsets[1].x = int32_t(this->width >> (i - 1));
        imageBlit.srcOffsets[1].y = int32_t(this->height >> (i - 1));
        imageBlit.srcOffsets[1].z = 1;

        // Destination
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = layer;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstOffsets[1].x = int32_t(this->width >> i);
        imageBlit.dstOffsets[1].y = int32_t(this->height >> i);
        imageBlit.dstOffsets[1].z = 1;

        VkImageSubresourceRange mipSubRange = {};
        mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        mipSubRange.baseMipLevel = i;
        mipSubRange.levelCount = 1;
        mipSubRange.layerCount = layer;

        // Prepare current mip level as image blit destination
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i, 1, cmd);

        // Blit from previous level
        vkCmdBlitImage(
            *cmd, this->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &imageBlit, VK_FILTER_LINEAR);

        // // Prepare current mip level as image blit source for next level
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i, 1, cmd);
    }
    if(extCmdBuffer == nullptr){
        cmd->endCommandBuffer();
        cmd->addRessourceToDestroy(cmd);
        cmd->submitCommandBuffer({}, {}, nullptr, true);
    }
}

void Image::createImage() {
    if (imageCreateInfo.enableMipMap) {
        imageCreateInfo.usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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
    std::cout << "Loading image: " << TEXTURE_PATH + filename[0] << std::endl;
    stbi_info((TEXTURE_PATH + filename[0]).c_str(), &width, &height, &nbOfchannel);
    if(width == 0 || height == 0){
        std::cerr << "Erreur: l'image n'a pas pu être chargée" << std::endl;
    }
    this->width = width;
    this->height = height;
    imageCreateInfo.width = width;
    imageCreateInfo.height = height;
    imageCreateInfo.layers = filename.size();
    this->layer = filename.size();
    for (size_t i = 0; i < filename.size(); i++) {
        imageCreateInfo.datas.push_back(stbi_load((TEXTURE_PATH + filename[i]).c_str(), &width, &height, &nbOfchannel, 4));
        
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
    cmdBuffer->addRessourceToDestroy(b);
    
    if (extCmdBuffer == nullptr) {
        cmdBuffer->endCommandBuffer();
        
        cmdBuffer->addRessourceToDestroy(cmdBuffer);
        cmdBuffer->submitCommandBuffer({}, {}, nullptr, true);
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

void Image::createsamplers(Device *device) {
    auto samplerInfo = make<VkSamplerCreateInfo>();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 8;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    auto result = vkCreateSampler(*device, &samplerInfo, nullptr, &linearSampler);
    if (result != VK_SUCCESS) {
        std::cerr << "Erreur: vkCreateSampler a échoué avec le code " << result << std::endl;
    }

    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    result = vkCreateSampler(*device, &samplerInfo, nullptr, &nearestSampler);
}

void Image::destroySamplers(Device *device) {
    vkDestroySampler(*device, linearSampler, nullptr);
    vkDestroySampler(*device, nearestSampler, nullptr);
}

}  // namespace TTe