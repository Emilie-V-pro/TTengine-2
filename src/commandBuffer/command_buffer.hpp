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
    CommandBufferPool(Device* device, const VkQueue& queue);
    // Destructor
    ~CommandBufferPool();
    // Copy/Move
    CommandBufferPool(const CommandBufferPool& cmdPoolHandler);
    CommandBufferPool& operator=(const CommandBufferPool& cmdPoolHandler);
    CommandBufferPool(CommandBufferPool&& cmdPoolHandler);
    CommandBufferPool& operator=(CommandBufferPool&& cmdPoolHandler);

    const VkCommandPool& operator()() const { return vk_cmdPool; }
    VkQueue queue() const { return vk_queue; }
    uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }

    std::vector<CommandBuffer> createCommandBuffer(unsigned int commandBufferCount);
    void resetPool();

    uint32_t cmdBufferCount = 0;

   private:
    VkQueue vk_queue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0;
    VkCommandPool vk_cmdPool = VK_NULL_HANDLE;
    Device* device = nullptr;


    friend class CommandBuffer;
};

class CommandBuffer : public vk_cmdBuffer_OBJ {
   public:
    CommandBuffer();
    CommandBuffer(Device* device, CommandBufferPool* CommandBufferPool, const VkCommandBuffer& cmdBuffer);
    // Destructor
    ~CommandBuffer();
    // Copy/Move
    CommandBuffer(const CommandBuffer& cmdBuffer) = delete;
    CommandBuffer(CommandBuffer&& cmdBuffer);
    CommandBuffer& operator=(const CommandBuffer& cmdPoolHandler) = delete;
    CommandBuffer& operator=(CommandBuffer&& cmdPoolHandler);

    operator VkCommandBuffer() const { return vk_cmdBuffer; }
    uint32_t getQueueFamilyIndex() const { return cmdBufferPool->queueFamilyIndex; }

    void beginCommandBuffer();
    void endCommandBuffer() const;
    void submitCommandBuffer(
        std::vector<VkSemaphoreSubmitInfo> waitSemaphores,
        std::vector<VkSemaphoreSubmitInfo> signalSemaphores,
        Fence* vk_fence = nullptr,
        bool waitForExecution = false);

    void addRessourceToDestroy(vk_cmdBuffer_OBJ* ressource);
    void addRessource(vk_cmdBuffer_OBJ &ressource) { 
        ressources.push_back(ressource);
        int x = 0;
        };

    bool fini = true;
   private:
    static void waitAndDestroy(CommandBuffer* cmdBuffer, Semaphore* s, uint32_t index);

    std::vector<vk_cmdBuffer_OBJ*> ressourcesToDestroy;
    std::vector<vk_cmdBuffer_OBJ> ressources;
    
    CommandBufferPool* cmdBufferPool = nullptr;

    VkCommandBuffer vk_cmdBuffer = VK_NULL_HANDLE;
    bool reseted = true;
    uint32_t index = 0;

    Device* device = nullptr;
    std::mutex mutex;

    friend class CommandBufferPool;
};

}  // namespace TTe