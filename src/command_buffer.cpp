
#include "command_buffer.hpp"

#include "structs_vk.hpp"
#include "synchronisation/fence.hpp"

// CommandBufferPool
namespace TTe {

CommandBufferPool::CommandBufferPool(const Device* device, const VkQueue& vk_queue) : vk_queue(vk_queue), device(device) {
    auto createInfo = make<VkCommandPoolCreateInfo>();
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = device->getRenderQueueFamilyIndexFromQueu(vk_queue);
    if (vkCreateCommandPool(device->device(), &createInfo, nullptr, &vk_cmdPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

CommandBufferPool::CommandBufferPool(const CommandBufferPool& cmdPool) : vk_queue(cmdPool.vk_queue), device(cmdPool.device) {
    auto createInfo = make<VkCommandPoolCreateInfo>();
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = device->getRenderQueueFamilyIndexFromQueu(vk_queue);
    if (vkCreateCommandPool(device->device(), &createInfo, nullptr, &vk_cmdPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

CommandBufferPool::CommandBufferPool(CommandBufferPool&& cmdPool) {
    vk_queue = cmdPool.vk_queue;
    device = cmdPool.device;
    vk_cmdPool = cmdPool.vk_cmdPool;
    commandBuffers = std::move(cmdPool.commandBuffers);

    cmdPool.vk_cmdPool = VK_NULL_HANDLE;
}

CommandBufferPool::~CommandBufferPool() {
    if (vk_cmdPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device->device(), vk_cmdPool, nullptr);
    }
    for (auto& cmdBuffer : commandBuffers) {
        cmdBuffer.vk_cmdBuffer = VK_NULL_HANDLE;
        cmdBuffer.cmdBufferPool = nullptr;
    }
}

CommandBufferPool& CommandBufferPool::operator=(const CommandBufferPool& cmdPoolHandler) {
    if (this != &cmdPoolHandler) {
        // Destroy the current command pool
        this->~CommandBufferPool();
        vk_queue = cmdPoolHandler.vk_queue;
        device = cmdPoolHandler.device;
        auto createInfo = make<VkCommandPoolCreateInfo>();
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        createInfo.queueFamilyIndex = device->getRenderQueueFamilyIndexFromQueu(vk_queue);
        if (vkCreateCommandPool(device->device(), &createInfo, nullptr, &vk_cmdPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool");
        }
    }
    return *this;
}

CommandBufferPool& CommandBufferPool::operator=(CommandBufferPool&& cmdPoolHandler) {
    if(this != &cmdPoolHandler) {
        this->~CommandBufferPool();
        commandBuffers = std::move(cmdPoolHandler.commandBuffers);
        vk_queue = cmdPoolHandler.vk_queue;
        device = cmdPoolHandler.device;
        vk_cmdPool = cmdPoolHandler.vk_cmdPool;
        cmdPoolHandler.vk_cmdPool = VK_NULL_HANDLE;
    }
    return *this;
}

std::vector<CommandBuffer> CommandBufferPool::createCommandBuffer(unsigned int commandBufferCount) const {
    std::vector<CommandBuffer> returnValue(commandBufferCount);
    std::vector<VkCommandBuffer> commandBuffers(commandBufferCount);

    auto allocInfo = make<VkCommandBufferAllocateInfo>();
    allocInfo.commandPool = vk_cmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBufferCount;
    if (vkAllocateCommandBuffers(device->device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    for (unsigned int i = 0; i < commandBufferCount; i++) {
        returnValue[i] = CommandBuffer(device, this, commandBuffers[i]);
    }
    return returnValue;
}

void CommandBufferPool::resetPool() {
    if (vkResetCommandPool(device->device(), vk_cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
        throw std::runtime_error("Failed to reset command pool");
    }
}
}

// CommandBuffer
namespace TTe {

CommandBuffer::CommandBuffer() {
}

CommandBuffer::CommandBuffer(const Device* device, const CommandBufferPool* CommandBufferPool, const VkCommandBuffer& cmdBuffer)
    :  cmdBufferPool(CommandBufferPool), vk_cmdBuffer(cmdBuffer), device(device) {}



CommandBuffer::CommandBuffer(CommandBuffer&& cmdBuffer) {
    cmdBufferPool = cmdBuffer.cmdBufferPool;
    vk_cmdBuffer = cmdBuffer.vk_cmdBuffer;
    device = cmdBuffer.device;
    ressourcesToDestroy = std::move(cmdBuffer.ressourcesToDestroy);

    cmdBuffer.vk_cmdBuffer = VK_NULL_HANDLE;
}

CommandBuffer::~CommandBuffer() {
    if (vk_cmdBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device->device(), cmdBufferPool->operator()(), 1, &vk_cmdBuffer);
    }
    for (auto& ressource : ressourcesToDestroy) {
        ressource->destroy();
    }
}


CommandBuffer& CommandBuffer::operator=(CommandBuffer&& cmdPoolHandler) {
    if(this != &cmdPoolHandler) {
        this->~CommandBuffer();

        ressourcesToDestroy = std::move(cmdPoolHandler.ressourcesToDestroy);
        cmdBufferPool = cmdPoolHandler.cmdBufferPool;
        vk_cmdBuffer = cmdPoolHandler.vk_cmdBuffer;
        device = cmdPoolHandler.device;

        cmdPoolHandler.vk_cmdBuffer = VK_NULL_HANDLE;
    }
    return *this;
}

void CommandBuffer::beginCommandBuffer() const {
    auto beginInfo = make<VkCommandBufferBeginInfo>();
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(vk_cmdBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
    }
}

void CommandBuffer::endCommandBuffer() const {
    if (vkEndCommandBuffer(vk_cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
}

void CommandBuffer::submitCommandBuffer(const std::vector<VkSemaphoreSubmitInfo> &waitSemaphores, const std::vector<VkSemaphoreSubmitInfo> &signalSemaphores, Fence &fence, bool waitForExecution) {
    auto cmdInfo = make<VkCommandBufferSubmitInfo>();
    cmdInfo.commandBuffer = this->vk_cmdBuffer;
    
    auto submitInfo = make<VkSubmitInfo2>();
    submitInfo.waitSemaphoreInfoCount = waitSemaphores.size();
    submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
    submitInfo.signalSemaphoreInfoCount = signalSemaphores.size();
    submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdInfo;

    

    if (vkQueueSubmit2(this->cmdBufferPool->queue(), 1, &submitInfo, fence()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit commandBuffer");
    }

    if(waitForExecution){
        
    }


}

void CommandBuffer::addRessourceToDestroy(Destroyable* ressource) {}

}  // namespace TTe