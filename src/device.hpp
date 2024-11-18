#pragma once

#include <volk.h>
#include <vulkan/vulkan_core.h>
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

    // Getter

   private:
    void createInstance();
    void selectPhysicalDevice(Window  &window);
    void createLogicialDevice();
    void setRequiredFeatures(vkb::PhysicalDeviceSelector &phys_device_selector);
    void setRequiredExtensions(vkb::PhysicalDeviceSelector &phys_device_selector);

    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkQueue renderQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;

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