#pragma once

#include <mutex>
#include <vector>

#include "../destroyable.hpp"
#include "../device.hpp"
#include "volk.h"
namespace TTe {
class Fence : public vk_cmdBuffer_OBJ {
   public:
    // Constructors
    Fence() = default;
    Fence(const Device *device, bool signaled = false);

    // Destructor
    ~Fence();

    // Copy and move constructors
    Fence(const Fence &other) = delete;
    Fence &operator=(const Fence &other) = delete;
    Fence(Fence &&other);
    Fence &operator=(Fence &&other);

    VkResult getFenceStatus();
    VkResult waitForFence();
    void resetFence();

    operator VkFence() const { return vk_Fence; }
    static VkResult waitForFences(const Device *vkDevice, const std::vector<Fence *> &fences, bool waitAllFence, int *firstFenceSignaled);

   private:
    VkFence vk_Fence = VK_NULL_HANDLE;
    std::mutex mutex;
    const Device *device = nullptr;
};
}  // namespace TTe