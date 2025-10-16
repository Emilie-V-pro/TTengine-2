#pragma once

#include <cstdint>
#include <vector>

#include "../destroyable.hpp"
#include "../device.hpp"
#include "volk.h"

namespace TTe {

class Semaphore : public vk_cmdBuffer_OBJ {
   public:
    // Constructor
    Semaphore() = default;
    Semaphore(Device *device, VkSemaphoreType vkSemaphoreType);

    // Destructor
    ~Semaphore();

    // Copy/Move
    Semaphore(const Semaphore &semaphore) = delete;
    Semaphore(Semaphore &&semaphore);

    Semaphore &operator=(const Semaphore &semaphore) = delete;
    Semaphore &operator=(Semaphore &&semaphore);

    uint64_t getTimeLineSemaphoreCountValue() const;
    VkResult waitTimeLineSemaphore(uint64_t waitValue) const;
    void signalTimeLineSemaphore(uint64_t signalValue) const;
    operator const VkSemaphore&() const { return vksemaphore; }

    VkSemaphoreSubmitInfo getSemaphoreSubmitSignalInfo();
    VkSemaphoreSubmitInfo getSemaphoreSubmitWaittInfo() const;
    uint32_t getTimelineValue() const { return timelineValue; };

    static VkResult waitTimeLineSemaphores(
        const Device *device, std::vector<const Semaphore *> semaphores, std::vector<uint64_t> waitValues, bool waitForFirstSemaphore);
    VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

   private:


    
    VkSemaphore vksemaphore = VK_NULL_HANDLE;
    uint32_t timelineValue = 0;
    VkSemaphoreType vkSemaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    const Device *device = nullptr;
};

}  // namespace TTe