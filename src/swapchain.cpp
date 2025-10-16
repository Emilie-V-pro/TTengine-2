
#include "swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "synchronisation/fence.hpp"
#include "volk.h"

#define VK_NO_PROTOTYPES

#include "VkBootstrap.h"
#include "structs_vk.hpp"
namespace TTe {

SwapChain::SwapChain(Device* p_device, const VkExtent2D p_window_extent, const vkb::SwapchainBuilder::BufferMode p_buffering_mode)
    : m_buffering_mode(p_buffering_mode), m_device(p_device) {
    vkb::SwapchainBuilder swapchain_builder{p_device->getVkbDevice()};
    swapchain_builder.set_desired_min_image_count(m_buffering_mode + 1);
    swapchain_builder.set_desired_extent(p_window_extent.width, p_window_extent.height);
    swapchain_builder.set_desired_format({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
    swapchain_builder.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR);
    auto swap_ret = swapchain_builder.build();

    if (!swap_ret) {
        throw std::runtime_error(swap_ret.error().message());
    }

    m_vkb_swapchain = swap_ret.value();
    m_number_of_frame = m_buffering_mode + 1;
    m_present_queue = p_device->getPresentQueue();
    generateSwapchainImages();
    createSyncObjects();
}

SwapChain::~SwapChain() {
    for (auto& fence : m_ressources_available_fences) {
        delete fence;
    }
    m_vkb_swapchain.destroy_image_views(m_swapchain_image_views);
    vkb::destroy_swapchain(m_vkb_swapchain);
}

void SwapChain::generateSwapchainImages() {
    auto vk_images = m_vkb_swapchain.get_images().value();
    m_swapchain_images.clear();
    m_swapchain_images.reserve(vk_images.size());

    m_swapchain_image_views = m_vkb_swapchain.get_image_views().value();

    for (uint32_t i = 0; i < m_vkb_swapchain.image_count; i++) {
        m_swapchain_images.emplace_back(
            m_device, vk_images[i], m_swapchain_image_views[i], m_vkb_swapchain.image_format, m_vkb_swapchain.extent);
        m_swapchain_images.back().name = "SC_" + std::to_string(i);
    }
}

void SwapChain::recreateSwapchain(const VkExtent2D p_window_extent) {
    m_vkb_swapchain.destroy_image_views(m_swapchain_image_views);
    vkb::SwapchainBuilder swapchain_builder{m_device->getVkbDevice()};
    swapchain_builder.set_old_swapchain(m_vkb_swapchain);
    swapchain_builder.set_desired_extent(p_window_extent.width, p_window_extent.height);
    swapchain_builder.set_desired_min_image_count(m_buffering_mode + 1);
    swapchain_builder.set_desired_format({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});

    swapchain_builder.set_image_usage_flags(
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

    auto swap_ret = swapchain_builder.build();
    if (!swap_ret) {
        throw std::runtime_error(swap_ret.error().message());
    }

    vkb::destroy_swapchain(m_vkb_swapchain);

    m_vkb_swapchain = swap_ret.value();
    for (auto& fence : m_ressources_available_fences) {
        delete fence;
    }
    generateSwapchainImages();
    createSyncObjects();
}

const VkResult SwapChain::acquireNextImage(
    uint32_t& p_current_swapchain_image, int* p_render_index, Semaphore*& p_aquire_frame_semaphore,  Fence*& p_fence) {
    Fence::waitForFences(m_device, m_ressources_available_fences, false, p_render_index);
    if (*p_render_index == -1) {
        return VK_TIMEOUT;
    }

    m_ressources_available_fences[*p_render_index]->resetFence();
    p_fence = m_ressources_available_fences[*p_render_index];

    VkResult result = vkAcquireNextImageKHR(
        *m_device, m_vkb_swapchain.swapchain, std::numeric_limits<uint64_t>::max(),
        m_image_available_semaphores[*p_render_index],  // must be a not signaled semaphore
        VK_NULL_HANDLE, &p_current_swapchain_image);

    m_image_available_semaphores[*p_render_index].stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    p_aquire_frame_semaphore = &m_image_available_semaphores[*p_render_index];

    return result;
}

const VkResult SwapChain::presentFrame(const uint32_t& p_current_swapchain_image, const Semaphore* p_wait_semaphore) const {
    auto present_info = make<VkPresentInfoKHR>();

    const VkSemaphore& test = *p_wait_semaphore;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &test;

    VkSwapchainKHR swapChains[] = {m_vkb_swapchain.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

    present_info.pImageIndices = &p_current_swapchain_image;

    auto result = vkQueuePresentKHR(m_present_queue, &present_info);
    return result;
}

void SwapChain::createSyncObjects() {
    m_ressources_available_fences.clear();
    m_image_available_semaphores.clear();
    m_ressources_available_fences.reserve(m_number_of_frame - 1);
    m_image_available_semaphores.reserve(m_number_of_frame - 1);
    for (unsigned int i = 0; i < m_number_of_frame - 1; i++) {
        m_ressources_available_fences.emplace_back(new Fence(m_device, true));
        m_image_available_semaphores.emplace_back(m_device, VK_SEMAPHORE_TYPE_BINARY);
        m_image_available_semaphores[i].stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    }
}
}  // namespace TTe