#pragma once


#include <vector>
#include "destroyable.hpp"
#include "device.hpp"
#include "synchronisation/fence.hpp"
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

    std::vector<CommandBuffer> createCommandBuffer(unsigned int commandBufferCount) const;
    void resetPool();
    VkQueue queue() const { return vk_queue;}


   private:
    std::vector<CommandBuffer> commandBuffers;
    VkQueue vk_queue = VK_NULL_HANDLE;
    VkCommandPool vk_cmdPool = VK_NULL_HANDLE;
    const Device* device = nullptr;
};



class CommandBuffer {
   public:
    CommandBuffer();
    CommandBuffer(const Device* device, const CommandBufferPool* CommandBufferPool, const VkCommandBuffer& cmdBuffer);
    // Destructor
    ~CommandBuffer();
    // Copy/Move
    CommandBuffer(const CommandBuffer& cmdBuffer) = delete;
    CommandBuffer(CommandBuffer&& cmdBuffer);
    CommandBuffer& operator=(const CommandBuffer& cmdPoolHandler) = delete;
    CommandBuffer& operator=(CommandBuffer&& cmdPoolHandler);

    
    const VkCommandBuffer &operator()() const { return vk_cmdBuffer; }


    void beginCommandBuffer() const;
    void endCommandBuffer() const;
    void submitCommandBuffer(const std::vector<VkSemaphoreSubmitInfo> &waitSemaphores, const std::vector<VkSemaphoreSubmitInfo> &signalSemaphores,  Fence &vk_fence, bool waitForExecution = false);

    void addRessourceToDestroy(Destroyable* ressource);
    static void waitAndDestroy(CommandBuffer & cmdBuffer, Fence &fence);
   private:
    
    std::vector<Destroyable*> ressourcesToDestroy;
    const CommandBufferPool* cmdBufferPool = nullptr;
    VkCommandBuffer vk_cmdBuffer = VK_NULL_HANDLE;
    const Device* device = nullptr;

    friend class CommandBufferPool;
};

}  // namespace TTe