
#include "swapchain.hpp"




#include <cstdint>
#include <vector>

#include "synchronisation/fence.hpp"
#include "volk.h"

#define VK_NO_PROTOTYPES

#include "VkBootstrap.h"
#include "structs_vk.hpp"
namespace TTe {

SwapChain::SwapChain(Device *device, VkExtent2D windowExtent, vkb::SwapchainBuilder::BufferMode bufferingMode)
    : bufferingMode(bufferingMode), device(device) {

    vkb::SwapchainBuilder swapchain_builder{device->getVkbDevice()};
    swapchain_builder.set_desired_min_image_count(bufferingMode + 1);
    swapchain_builder.set_desired_extent(windowExtent.width, windowExtent.height);
    swapchain_builder.set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
    swapchain_builder.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    // swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
    auto swap_ret = swapchain_builder.build();

    if (!swap_ret) {
        throw std::runtime_error(swap_ret.error().message());
    }

    vkbSwapchain = swap_ret.value();
    numberOfFrame = bufferingMode + 1;
    presentQueue = device->getPresentQueue();
    generateSwapchainImages();
    createSyncObjects();
}

SwapChain::~SwapChain() {
    vkbSwapchain.destroy_image_views(swapChainImageView);
    vkb::destroy_swapchain(vkbSwapchain);
}

void SwapChain::generateSwapchainImages() {
    auto vkImages = vkbSwapchain.get_images().value();
    swapChainImage.clear();
    swapChainImage.reserve(vkImages.size());

    swapChainImageView = vkbSwapchain.get_image_views().value();

    for (uint32_t i = 0; i < vkbSwapchain.image_count; i++) {
        swapChainImage.emplace_back(device, vkImages[i], swapChainImageView[i], vkbSwapchain.image_format, vkbSwapchain.extent);
    }
}

void SwapChain::recreateSwapchain(VkExtent2D windowExtent) {

    vkbSwapchain.destroy_image_views(swapChainImageView);
    vkb::SwapchainBuilder swapchain_builder{device->getVkbDevice()};
    swapchain_builder.set_old_swapchain(vkbSwapchain);
    swapchain_builder.set_desired_extent(windowExtent.width, windowExtent.height);
    swapchain_builder.set_desired_min_image_count(bufferingMode + 1);
    swapchain_builder.set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
    swapchain_builder.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    auto swap_ret = swapchain_builder.build();
    if (!swap_ret) {
        throw std::runtime_error(swap_ret.error().message());
    }

    vkb::destroy_swapchain(vkbSwapchain);

    vkbSwapchain = swap_ret.value();
    generateSwapchainImages();
    createSyncObjects();
}


VkResult SwapChain::acquireNextImage(uint32_t& currentSwapchainImage, Semaphore* aquireFrameSemaphore) {
    int frameIndex = 0;
    Fence::waitForFences(device, imageAvailableFences, false, &frameIndex);
    if (frameIndex > static_cast<int>(imageAvailableFences.size())) {
        return VK_TIMEOUT;
    }

    imageAvailableFences[frameIndex]->resetFence();

    VkResult result = vkAcquireNextImageKHR(
        *device, vkbSwapchain.swapchain, std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[frameIndex],  // must be a not signaled semaphore
        VK_NULL_HANDLE, &currentSwapchainImage);
        
    imageAvailableSemaphores[frameIndex].signalStage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    aquireFrameSemaphore = &imageAvailableSemaphores[frameIndex];
    

    return result;
}

VkResult SwapChain::presentFrame(uint32_t& currentSwapchainImage, std::vector<Semaphore*> waitSemaphores) {
    auto presentInfo = make<VkPresentInfoKHR>();

    std::vector<VkSemaphore> waitVKSemaphores(waitSemaphores.size());
    for (size_t i = 0; i < waitSemaphores.size(); i++) {
        waitVKSemaphores[i] = *waitSemaphores[i];
    }

    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitVKSemaphores.data();

    VkSwapchainKHR swapChains[] = {vkbSwapchain.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &currentSwapchainImage;

    auto result = vkQueuePresentKHR(presentQueue, &presentInfo);
    return result;
}

void SwapChain::createSyncObjects() {
    imageAvailableFences.clear();
    imageAvailableSemaphores.clear();
    imageAvailableFences.reserve(numberOfFrame - 1);
    imageAvailableSemaphores.reserve(numberOfFrame - 1);
    for (unsigned int i = 0; i < numberOfFrame - 1; i++) {
        imageAvailableFences.emplace_back(new Fence(device, true));
        imageAvailableSemaphores.emplace_back(device, VK_SEMAPHORE_TYPE_BINARY);
    }
}
}  // namespace TTe