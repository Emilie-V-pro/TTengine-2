#pragma once

#include <cstdint>
#include <vector>

#include "commandBuffer/command_buffer.hpp"
#include "destroyable.hpp"
#include "device.hpp"

namespace TTe {

struct ImageCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers = 1;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags usageFlags = 0;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool isCubeTexture = false;
    bool enableMipMap = false;
    std::vector<std::string> filename;
    std::vector<void *> datas;
};

class Image : Destroyable {
   public:
    // Constructor
    Image(Device *device, ImageCreateInfo &imageCreateInfo, CommandBuffer *cmdBuffer = nullptr);
    Image(Device *device, VkImage image, VkImageView imageView, VkFormat format, VkExtent2D swapChainExtent);

    // Destructor
    ~Image();

    // Copy and move constructors
    Image(Image &other);
    Image &operator=(Image &other);
    Image(Image &&other);
    Image &operator=(Image &&other);

    // Getters
    operator VkImage() const { return image; }
    operator VkImageView() const { return imageView; }
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

    void transitionImageLayout(VkImageLayout newLayout, CommandBuffer *cmdBuffer = nullptr);
    void transitionImageLayout(
        VkImageLayout oldLayout, VkImageLayout newLayout, u_int32_t mipLevel, u_int32_t mipCount, CommandBuffer *extCmdBuffer = nullptr);

    void generateMipmaps();

    static void copyImage(Device *device, Image &srcImage, Image &dstImage, CommandBuffer *extCmdBuffer = nullptr);

   private:
    void createImage();
    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties);
    void createImageView();
    void loadImageFromFile(std::vector<std::string> &filename);
    void loadImageToGPU(CommandBuffer *extCmdBuffer = nullptr);

    ImageCreateInfo imageCreateInfo;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layer = 0;
    uint32_t mipLevels = 1;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout actualImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    Device *device = nullptr;

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;

    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;

    bool isSwapchainImage = false;
};
}  // namespace TTe