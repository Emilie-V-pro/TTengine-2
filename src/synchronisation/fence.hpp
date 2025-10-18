#pragma once

#include <mutex>
#include <vector>

#include "../destroyable.hpp"
#include "../device.hpp"
#include "volk.h"
namespace TTe {
class Fence : public CmdBufferRessource {
   public:
    // Constructors
    Fence() = default;
    Fence(const Device *p_device, bool p_signaled = false);

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

    operator VkFence() const { return m_vk_fence; }
    static VkResult waitForFences(const Device *p_device, const std::vector<Fence *> &p_fences, bool p_wait_all_fence, int *p_first_fence_signaled);

   private:
    VkFence m_vk_fence = VK_NULL_HANDLE;
    std::mutex m_mutex;
    const Device *m_device = nullptr;
};
}  // namespace TTe