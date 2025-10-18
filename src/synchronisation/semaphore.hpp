#pragma once

#include <cstdint>
#include <vector>

#include "../destroyable.hpp"
#include "../device.hpp"
#include "volk.h"

namespace TTe {

class Semaphore : public CmdBufferRessource {
   public:
    // Constructor
    Semaphore() = default;
    Semaphore(Device *p_device, VkSemaphoreType p_vk_semaphore_type);

    // Destructor
    ~Semaphore();

    // Copy/Move
    Semaphore(const Semaphore &other) = delete;
    Semaphore(Semaphore &&other);

    Semaphore &operator=(const Semaphore &other) = delete;
    Semaphore &operator=(Semaphore &&other);

    uint64_t getTimeLineSemaphoreCountValue() const;
    VkResult waitTimeLineSemaphore(uint64_t p_wait_value) const;
    void signalTimeLineSemaphore(uint64_t p_signal_value) const;
    operator const VkSemaphore&() const { return m_vk_semaphore; }

    VkSemaphoreSubmitInfo getSemaphoreSubmitSignalInfo();
    VkSemaphoreSubmitInfo getSemaphoreSubmitWaittInfo() const;
    uint32_t getTimelineValue() const { return m_timeline_value; };

    static VkResult waitTimeLineSemaphores(
        const Device *p_device,  const Semaphore * p_semaphores, const uint64_t * p_wait_values, const uint32_t p_semaphore_count, bool p_wait_for_first_semaphore);
    
        VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

   private:


    
    VkSemaphore m_vk_semaphore = VK_NULL_HANDLE;
    uint32_t m_timeline_value = 0;
    VkSemaphoreType m_vk_semaphore_type = VK_SEMAPHORE_TYPE_BINARY;
    const Device *m_device = nullptr;
};

}  // namespace TTe