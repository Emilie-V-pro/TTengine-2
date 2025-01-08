#pragma once

#include <volk.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"
#include "window.hpp"
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

    //get device
    const VkDevice& operator()() const { return vkDevice; }
    const VkDevice& device() const { return vkDevice; }

   private:
    void createInstance();
    void selectPhysicalDevice(Window  &window);
    void createLogicialDevice();
    void setRequiredFeatures(vkb::PhysicalDeviceSelector &phys_device_selector);
    void setRequiredExtensions(vkb::PhysicalDeviceSelector &phys_device_selector);

    VkDevice vkDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkQueue renderQueue;
    uint32_t renderQueueFamilyIndex;
    VkQueue computeQueue;
    uint32_t computeQueueFamilyIndex;
    VkQueue transferQueue;
    uint32_t transferQueueFamilyIndex;

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