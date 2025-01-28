#pragma once

#include <volk.h>
#include <cstdint>
#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"
#include "window.hpp"



#include "vk_mem_alloc.h"


namespace TTe {
class Device {
   public:
    Device(Window &window);
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

    uint32_t getRenderQueueFamilyIndexFromQueu(const VkQueue& queue) const  { 
        if(queue == renderQueue) return renderQueueFamilyIndex;
        if(queue == computeQueue) return computeQueueFamilyIndex;
        if(queue == transferQueue) return transferQueueFamilyIndex;
        return -1;
     }

    const VkQueue& getRenderQueue() const { return renderQueue; }
    const VkQueue& getComputeQueue() const { return computeQueue; }
    const VkQueue& getTransferQueue() const { return transferQueue; }
    const VkQueue& getPresentQueue() const { return presentQueue; }

    const VmaAllocator& getAllocator() const { return _allocator; } 

    const vkb::Device &getVkbDevice() const { return vkbDevice; }

    //get device
    operator VkDevice() const { return vk_device; }

   private:
    void createInstance();
    void selectPhysicalDevice(Window  &window);
    void createLogicialDevice();
    void setRequiredFeatures(vkb::PhysicalDeviceSelector &phys_device_selector);
    void setRequiredExtensions(vkb::PhysicalDeviceSelector &phys_device_selector);
    void initVMA();

    VkDevice vk_device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VmaAllocator _allocator ;
     VmaVulkanFunctions vma_vulkan_func{};

    VkQueue renderQueue = VK_NULL_HANDLE;
    uint32_t renderQueueFamilyIndex = -1;
    VkQueue computeQueue = VK_NULL_HANDLE;
    uint32_t computeQueueFamilyIndex = -1;
    VkQueue transferQueue = VK_NULL_HANDLE;
    uint32_t transferQueueFamilyIndex = -1;

    VkQueue presentQueue = VK_NULL_HANDLE;

    vkb::Instance vkbInstance;
    vkb::PhysicalDevice vkbPhysicalDevice;
    vkb::Device vkbDevice;

    // disable validation on release build type to get full performance
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
}  // namespace TTe