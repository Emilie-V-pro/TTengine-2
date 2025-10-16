#pragma once

#include <cstdint>
#include <vector>

#include "GPU_data/image.hpp"
#include "volk.h"

#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"
#include "device.hpp"
#include "synchronisation/fence.hpp"
#include "synchronisation/semaphore.hpp"
namespace TTe {

class Swapchain {
   public:
    // constructor
    Swapchain(Device* p_device, const VkExtent2D p_window_extent, const vkb::SwapchainBuilder::BufferMode p_buffering_mode);

    // destructor
    ~Swapchain();

    // delete copy and move constructors
    Swapchain(const Swapchain&) = delete;
    Swapchain(Swapchain&&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain& operator=(Swapchain&&) = delete;

    void recreateSwapchain(const VkExtent2D p_window_extent);

    VkResult acquireNextImage(uint32_t& p_current_swapchain_image, int* p_render_index,  Semaphore*& p_aquire_frame_semaphore,  Fence*& p_fence);
    VkResult presentFrame(const uint32_t& p_current_swapchain_image, const Semaphore* p_wait_semaphore) const;


    Image& getSwapChainImage(const unsigned int p_index)  { return m_swapchain_images[p_index]; }
    std::vector<Image>& getswapChainImages()  { return m_swapchain_images; };

   private:
    void generateSwapchainImages();
    void createSyncObjects();

    unsigned int m_number_of_frame = 0;

    std::vector<Image> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;

    // should be mouved out of swapchain
    std::vector<Fence*> m_ressources_available_fences;
    std::vector<Semaphore> m_image_available_semaphores;

    vkb::SwapchainBuilder::BufferMode m_buffering_mode;

    VkQueue m_present_queue = VK_NULL_HANDLE;
    Device* m_device = nullptr;
    vkb::Swapchain m_vkb_swapchain;
};

}  // namespace TTe