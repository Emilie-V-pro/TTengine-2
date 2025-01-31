#pragma once

#include <vector>
#include <chrono>

#include "image.hpp"
#include "volk.h"

#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"
#include "device.hpp"
#include "synchronisation/fence.hpp"
#include "synchronisation/semaphore.hpp"
#include "utils.hpp"
namespace TTe {

class SwapChain {
   public:
    // constructor
    SwapChain(Device *device, VkExtent2D windowExtent, vkb::SwapchainBuilder::BufferMode bufferingMode);

    // destructor
    ~SwapChain();

    //delete copy and move constructors
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    void recreateSwapchain(VkExtent2D windowExtent);

    VkResult acquireNextImage(uint32_t& currentSwapchainImage, Semaphore* aquireFrameSemaphore);
    VkResult presentFrame(uint32_t& currentSwapchainImage, std::vector<Semaphore*> waitSemaphores);
   
    float extentAspectRatio() { 
        return static_cast<float>(vkbSwapchain.extent.width) / static_cast<float>(vkbSwapchain.extent.height); 
        }

    Image& getSwapChainImage(unsigned int index) { return swapChainImage[index]; }

    std::vector<Image> &getswapChainImages(){return swapChainImage;};

   private:
    void generateSwapchainImages();
    void createSyncObjects();

    unsigned int numberOfFrame = 0;

    std::vector<Image> swapChainImage;
    std::vector<VkImageView> swapChainImageView;

    std::vector<Fence *> imageAvailableFences;
    std::vector<Semaphore> imageAvailableSemaphores;
    
    vkb::SwapchainBuilder::BufferMode bufferingMode;


    VkQueue presentQueue = VK_NULL_HANDLE;
    Device *device = nullptr;
    vkb::Swapchain vkbSwapchain;
};

}  // namespace vk_stage