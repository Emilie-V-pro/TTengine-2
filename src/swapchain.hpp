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

class SwapChain {
   public:
    // constructor
    SwapChain(Device* p_device, const VkExtent2D p_window_extent, const vkb::SwapchainBuilder::BufferMode p_buffering_mode);

    // destructor
    ~SwapChain();

    // delete copy and move constructors
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    void recreateSwapchain(const VkExtent2D p_window_extent);

    const VkResult acquireNextImage(uint32_t& p_current_swapchain_image, int* p_render_index,  Semaphore*& p_aquire_frame_semaphore,  Fence*& p_fence);
    const VkResult presentFrame(const uint32_t& p_current_swapchain_image, const Semaphore* p_wait_semaphore) const;

    const float getExtentAspectRatio() const {
        return static_cast<float>(m_vkb_swapchain.extent.width) / static_cast<float>(m_vkb_swapchain.extent.height);
    }

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