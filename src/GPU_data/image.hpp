#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>


#include "commandBuffer/command_buffer.hpp"
#include "destroyable.hpp"
#include "device.hpp"

namespace TTe {

enum samplerType { LINEAR, NEAREST };

struct ImageCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers = 1;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags usageFlags = 0;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool isCubeTexture = false;
    bool enableMipMap = false;
    std::vector<std::filesystem::path> filename;
    std::vector<void *> datas;
};

class Image : public CmdBufferRessource {
   public:
    // Constructor
    Image(){};
    Image(Device *device, ImageCreateInfo &imageCreateInfo, CommandBuffer *cmdBuffer = nullptr);
    Image(Device *device, VkImage image, VkImageView imageView, VkFormat format, VkExtent2D swapChainExtent);

    // Destructor
    ~Image();

    // Copy and move constructors
    Image(const Image &other);
    Image &operator=(const Image &other);
    Image(Image &&other);
    Image &operator=(Image &&other);

    // Getters
    operator VkImage() const { return vk_image; }
    operator VkImageView() const { return imageView; }
    operator VkImageLayout() const { return actualImageLayout; }

    VkFormat getFormat() const { return imageFormat; }
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    VkDescriptorImageInfo getDescriptorImageInfo() const {
        return {VK_NULL_HANDLE, imageView, imageLayout};
    }
    VkDescriptorImageInfo getDescriptorImageInfo(samplerType type) const {
        VkSampler sampler;
        switch (type) {
            case LINEAR:
                sampler = linearSampler;
                break;
            case NEAREST:
                sampler = nearestSampler;
                break;
            default:
                sampler = VK_NULL_HANDLE;
                break;
        
        }

        return {sampler, imageView, imageLayout};
    }
    
    bool isSwapchainImg() const { return isSwapchainImage; }


    void writeToImage(void *data, size_t size, uint32_t offset = 0, CommandBuffer *extCmdBuffer = nullptr);

    void transitionImageLayout(VkImageLayout newLayout, CommandBuffer *cmdBuffer = nullptr);
    void transitionImageLayout(
        VkImageLayout oldLayout, VkImageLayout newLayout, u_int32_t mipLevel, u_int32_t mipCount, CommandBuffer *extCmdBuffer = nullptr);

    void addImageMemoryBarrier(const CommandBuffer &extCmdBuffer, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask);
    void transferQueueOwnership(const CommandBuffer &extCmdBuffer, uint32_t queueIndex);

    void generateMipmaps(CommandBuffer *extCmdBuffer = nullptr);

    static void blitImage(Device *device, Image &srcImage, Image &dstImage, CommandBuffer *extCmdBuffer = nullptr);
    static void copyImage(Device *device, Image &srcImage, Image &dstImage, CommandBuffer *extCmdBuffer = nullptr);

    void static createsamplers(Device *device);
    void static destroySamplers(Device *device);

    std::string name;

    void saveImageToFile();

   private:
    void createImage();
    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties);
    void createImageView();
    void loadImageFromFile(std::vector<std::filesystem::path> &filename);
    void loadImageToGPU(CommandBuffer *extCmdBuffer = nullptr);
    void destruction();

    ImageCreateInfo imageCreateInfo;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layer = 0;
    uint32_t mipLevels = 1;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout actualImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    Device *device = nullptr;

    VkImage vk_image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;

    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;

   
    std::atomic<std::shared_ptr<int>> refCount;

    static VkSampler linearSampler;
    static VkSampler nearestSampler;

    bool isSwapchainImage = false;
};
}  // namespace TTe