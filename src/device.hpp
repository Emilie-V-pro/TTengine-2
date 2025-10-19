#pragma once

#include <volk.h>

#include <cstdint>
#include <mutex>
#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"
#include "window.hpp"


// #define VMA_DEBUG_LOG(format, ...) printf(format "\n", ##__VA_ARGS__)
#include "vk_mem_alloc.h"


namespace TTe {
class Device {
   public:
    Device(Window &p_window);
    ~Device();

    const VkDevice &operator()();

    /**
     * Only one device object should be created for this engine.
     */
    // desable copy
    Device(Device const &) = delete;
    Device &operator=(Device const &) = delete;
    // desable move
    Device(Device &&) = delete;
    Device &operator=(Device &&) = delete;

    uint32_t getRenderQueueFamilyIndexFromQueu(const VkQueue& p_queue) const  { 
        if(p_queue == m_render_queue) return m_render_queue_family_index;
        if(p_queue == m_compute_queue) return m_compute_queue_family_index;
        if(p_queue == m_transfer_queue) return m_transfer_queue_family_index;
        return -1;
     }

    std::mutex &getMutexFromQueue(const VkQueue& p_queue)  { 
        if(p_queue == m_render_queue) return m_render_queue_mutex;
        if(p_queue == m_compute_queue) return m_compute_queue_mutex;
        if(p_queue == m_transfer_queue) return m_transfer_queue_mutex;
        throw std::runtime_error("Invalid queue");
     }

    VkInstance getInstance() const { return m_vk_instance; }

    const VkQueue& getRenderQueue() const { return m_render_queue; }
    const VkQueue& getComputeQueue() const { return m_compute_queue; }
    const VkQueue& getTransferQueue() const { return m_transfer_queue; }
    const VkQueue& getPresentQueue() const { return m_present_queue; }

    const VmaAllocator& getAllocator() const { return m_allocator; } 

    const vkb::Device &getVkbDevice() const { return m_vkb_device; }

    const VkPhysicalDeviceDescriptorBufferPropertiesEXT &getDeviceDescProps() const { return m_device_desc_props; }
    VkFormat findSupportedFormat(const std::vector<VkFormat> &p_candidates, VkImageTiling p_tiling, VkFormatFeatureFlags p_features);

    //get device
    operator VkDevice() const { return m_vk_device; }

   private:
    void createInstance();
    void selectPhysicalDevice(Window  &p_window);
    void createLogicialDevice();
    void setRequiredFeatures(vkb::PhysicalDeviceSelector &p_phys_device_selector);
    void setRequiredExtensions(vkb::PhysicalDeviceSelector &p_phys_device_selector);
    void initVMA();

    void queryPhysicalDeviceProperties();

    VkDevice m_vk_device = VK_NULL_HANDLE;
    VkInstance m_vk_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VmaAllocator m_allocator ;
    VmaVulkanFunctions m_vma_vulkan_func{};

    VkQueue m_render_queue = VK_NULL_HANDLE;
    uint32_t m_render_queue_family_index = -1;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
    uint32_t m_compute_queue_family_index = -1;
    VkQueue m_transfer_queue = VK_NULL_HANDLE;
    uint32_t m_transfer_queue_family_index = -1;

    std::mutex m_render_queue_mutex;
    std::mutex m_compute_queue_mutex;
    std::mutex m_transfer_queue_mutex;

    VkQueue m_present_queue = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties2KHR m_device_props_2 = {};
    VkPhysicalDeviceDescriptorBufferPropertiesEXT m_device_desc_props = {};

    vkb::Instance m_vkb_instance;
    vkb::PhysicalDevice m_vkb_physical_device;
    vkb::Device m_vkb_device;

    // disable validation on release build m_type to get full performance
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
}  // namespace TTe