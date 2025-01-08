#pragma once

#include <cstdint>
#include <vector>

#include "../device.hpp"
#include "volk.h"

namespace TTe {

class Semaphore {
   public:
    // Constructor
    Semaphore(){};
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
    VkSemaphore semaphore() const { return vksemaphore; }

    static VkResult waitTimeLineSemaphores(
        const Device *device, std::vector<const Semaphore*> semaphores, std::vector<uint64_t> waitValues, bool waitForFirstSemaphore);

   private:
    VkSemaphore vksemaphore = VK_NULL_HANDLE;
    const VkSemaphoreType vkSemaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    const Device *device = nullptr;
};

}  // namespace TTe