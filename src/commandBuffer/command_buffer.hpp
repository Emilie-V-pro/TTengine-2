#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

#include "../destroyable.hpp"
#include "../device.hpp"
#include "../synchronisation/fence.hpp"
#include "synchronisation/semaphore.hpp"
#include "volk.h"

namespace TTe {
class CommandBuffer;

class CommandBufferPool {
   public:
    // Constructor
    CommandBufferPool() = default;
    CommandBufferPool(Device* p_device, const VkQueue& p_queue);
    // Destructor
    ~CommandBufferPool();
    // Copy/Move
    CommandBufferPool(const CommandBufferPool& other);
    CommandBufferPool& operator=(const CommandBufferPool& other);
    CommandBufferPool(CommandBufferPool&& other);
    CommandBufferPool& operator=(CommandBufferPool&& other);

    const VkCommandPool& operator()() const { return m_vk_cmd_pool; }
    VkQueue queue() const { return m_vk_queue; }
    uint32_t getQueueFamilyIndex() const { return m_queue_family_index; }

    std::vector<CommandBuffer> createCommandBuffer(unsigned int commandBufferCount);
    void resetPool();

    uint32_t cmd_buffer_count = 0;

   private:
    VkQueue m_vk_queue = VK_NULL_HANDLE;
    uint32_t m_queue_family_index = 0;
    VkCommandPool m_vk_cmd_pool = VK_NULL_HANDLE;
    Device* m_device = nullptr;


    friend class CommandBuffer;
};

class CommandBuffer : public CmdBufferRessource {
   public:
    CommandBuffer();
    CommandBuffer(Device* p_device, CommandBufferPool* p_command_buffer_pool, const VkCommandBuffer& p_cmd_buffer);
    // Destructor
    ~CommandBuffer();
    // Copy/Move
    CommandBuffer(const CommandBuffer& other) = delete;
    CommandBuffer& operator=(const CommandBuffer& other) = delete;
    CommandBuffer(CommandBuffer&& other);
    CommandBuffer& operator=(CommandBuffer&& other);

    operator VkCommandBuffer() const { return m_vk_cmd_buffer; }
    uint32_t getQueueFamilyIndex() const { return m_cmd_buffer_pool->m_queue_family_index; }

    void beginCommandBuffer();
    void endCommandBuffer() const;
    void submitCommandBuffer(
        std::vector<VkSemaphoreSubmitInfo> p_wait_semaphores,
        std::vector<VkSemaphoreSubmitInfo> p_signal_semaphores,
        Fence* p_vk_fence = nullptr,
        bool p_wait_for_execution = false);

    void addRessourceToDestroy(CmdBufferRessource* p_ressource);
    void addRessource(CmdBufferRessource &p_ressource);

    bool fini = true;
   private:
    static void waitAndDestroy(CommandBuffer* p_cmd_buffer, Semaphore* p_semaphore, uint32_t p_index);

    std::vector<CmdBufferRessource*> m_ressources_to_destroy;
    std::vector<CmdBufferRessource> m_ressources;
    
    CommandBufferPool* m_cmd_buffer_pool = nullptr;

    VkCommandBuffer m_vk_cmd_buffer = VK_NULL_HANDLE;
    bool m_reseted = true;
    uint32_t m_index = 0;

    Device* m_device = nullptr;
    std::mutex m_mutex;

    friend class CommandBufferPool;
};

}  // namespace TTe