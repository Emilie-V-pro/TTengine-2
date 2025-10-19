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
    VkImageUsageFlags usage_flags = 0;
    VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool is_cube_texture = false;
    bool enable_mipmap = false;
    std::vector<std::filesystem::path> filename;
    std::vector<void *> datas;
};

class Image : public CmdBufferRessource {
   public:
    // Constructor
    Image(){};
    Image(Device *p_device, ImageCreateInfo &p_image_create_info, CommandBuffer *p_cmdBuffer = nullptr);
    Image(Device *p_device, VkImage p_image, VkImageView p_image_view, VkFormat p_format, VkExtent2D p_swap_chain_extent);

    // Destructor
    ~Image();

    // Copy and move constructors
    Image(const Image &other);
    Image &operator=(const Image &other);
    Image(Image &&other);
    Image &operator=(Image &&other);

    // Getters
    operator VkImage() const { return m_vk_image; }
    operator VkImageView() const { return m_image_view; }
    operator VkImageLayout() const { return m_actual_image_layout; }

    VkFormat getFormat() const { return m_image_format; }
    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }
    VkDescriptorImageInfo getDescriptorImageInfo() const {
        return {VK_NULL_HANDLE, m_image_view, m_image_layout};
    }
    VkDescriptorImageInfo getDescriptorImageInfo(samplerType p_type) const {
        VkSampler sampler;
        switch (p_type) {
            case LINEAR:
                sampler = s_linear_sampler;
                break;
            case NEAREST:
                sampler = s_nearest_sampler;
                break;
            default:
                sampler = VK_NULL_HANDLE;
                break;
        
        }

        return {sampler, m_image_view, m_image_layout};
    }
    
    bool isSwapchainImg() const { return m_is_swapchain_image; }


    void writeToImage(void *p_data, size_t p_size, uint32_t p_offset = 0, CommandBuffer *p_ext_cmd_buffer = nullptr);

    void transitionImageLayout(VkImageLayout p_new_layout, CommandBuffer *p_cmd_buffer = nullptr);
    void transitionImageLayout(
        VkImageLayout p_old_layout, VkImageLayout p_new_layout, u_int32_t p_mip_level, u_int32_t p_mip_count, CommandBuffer *p_ext_cmd_buffer = nullptr);

    void addImageMemoryBarrier(const CommandBuffer &p_ext_cmd_buffer, VkPipelineStageFlags2 p_src_stage_mask, VkPipelineStageFlags2 p_dst_stage_mask);
    void transferQueueOwnership(const CommandBuffer &p_ext_cmd_buffer, uint32_t p_queue_index);

    void generateMipmaps(CommandBuffer *p_ext_cmd_buffer = nullptr);

    static void blitImage(Device *p_device, Image &p_src_image, Image &p_dst_image, CommandBuffer *p_ext_cmd_buffer = nullptr);
    static void copyImage(Device *p_device, Image &p_src_image, Image &p_dst_image, CommandBuffer *p_ext_cmd_buffer = nullptr);

    void static createsamplers(Device *p_device);
    void static destroySamplers(Device *p_device);

    void saveImageToFile();
    
    std::string name;

   private:
    void createImage();
    void createImageWithInfo(const VkImageCreateInfo &p_image_info, VkMemoryPropertyFlags p_properties);
    void createImageView();
    void loadImageFromFile(std::vector<std::filesystem::path> &p_filename);
    void loadImageToGPU(CommandBuffer *p_ext_cmd_buffer = nullptr);
    void destruction();

    ImageCreateInfo m_image_create_info;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_layer = 0;
    uint32_t m_mip_levels = 1;
    VkFormat m_image_format = VK_FORMAT_UNDEFINED;
    VkImageLayout m_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout m_actual_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    Device *m_device = nullptr;

    VkImage m_vk_image = VK_NULL_HANDLE;
    VkImageView m_image_view = VK_NULL_HANDLE;

    VmaAllocation m_allocation = VK_NULL_HANDLE;

   
    std::atomic<std::shared_ptr<int>> m_ref_count;

    static VkSampler s_linear_sampler;
    static VkSampler s_nearest_sampler;

    bool m_is_swapchain_image = false;
};
}  // namespace TTe