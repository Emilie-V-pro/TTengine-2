#pragma once

#include <cstdint>
#include <vector>
#include "../destroyable.hpp"
#include "../device.hpp"
#include "../synchronisation/fence.hpp"
#include "volk.h"
namespace TTe {
class CommandBuffer;

class CommandBufferPool {
   public:
    // Constructor
    CommandBufferPool() = default;
    CommandBufferPool(const Device* device, const VkQueue& queue);
    // Destructor
    ~CommandBufferPool();
    // Copy/Move
    CommandBufferPool(const CommandBufferPool& cmdPoolHandler);
    CommandBufferPool& operator=(const CommandBufferPool& cmdPoolHandler);
    CommandBufferPool(CommandBufferPool&& cmdPoolHandler);
    CommandBufferPool& operator=(CommandBufferPool&& cmdPoolHandler);

    const VkCommandPool &operator()() const { return vk_cmdPool; }
    VkQueue queue() const { return vk_queue;}
    unsigned int getNBCmBuffers() const { return nbCommandBuffers; }
    uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }

    std::vector<CommandBuffer> createCommandBuffer(unsigned int commandBufferCount) ;
    void resetPool();

   private:

    unsigned int nbCommandBuffers = 0;
    VkQueue vk_queue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0;
    VkCommandPool vk_cmdPool = VK_NULL_HANDLE;
    const Device* device = nullptr;

    friend class CommandBuffer;
};



class CommandBuffer : public Destroyable {
   public:
    CommandBuffer();
    CommandBuffer(const Device* device, CommandBufferPool* CommandBufferPool, const VkCommandBuffer& cmdBuffer);
    // Destructor
    ~CommandBuffer();
    // Copy/Move
    CommandBuffer(const CommandBuffer& cmdBuffer) = delete;
    CommandBuffer(CommandBuffer&& cmdBuffer);
    CommandBuffer& operator=(const CommandBuffer& cmdPoolHandler) = delete;
    CommandBuffer& operator=(CommandBuffer&& cmdPoolHandler);

    
    operator VkCommandBuffer() const { return vk_cmdBuffer; }
    uint32_t getQueueFamilyIndex() const { return cmdBufferPool->queueFamilyIndex; }


    void beginCommandBuffer() const;
    void endCommandBuffer() const;
    void submitCommandBuffer(const std::vector<VkSemaphoreSubmitInfo> &waitSemaphores, const std::vector<VkSemaphoreSubmitInfo> &signalSemaphores,  Fence* vk_fence = nullptr, bool waitForExecution = false);

    void addRessourceToDestroy(Destroyable* ressource);
   private:
    
    static void waitAndDestroy(CommandBuffer * cmdBuffer, Fence *fence);
    std::vector<Destroyable*> ressourcesToDestroy;
    CommandBufferPool* cmdBufferPool = nullptr;
    VkCommandBuffer vk_cmdBuffer = VK_NULL_HANDLE;
    const Device* device = nullptr;

    friend class CommandBufferPool;
};

}  // namespace TTe