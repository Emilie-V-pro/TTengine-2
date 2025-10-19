
#include "command_buffer.hpp"

#include <cstdint>
#include <thread>

#include "../synchronisation/fence.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "structs_vk.hpp"
#include "synchronisation/semaphore.hpp"

// CommandBufferPool
namespace TTe {

CommandBufferPool::CommandBufferPool(Device* p_device, const VkQueue& p_queue) : m_vk_queue(p_queue), m_device(p_device) {
    auto create_info = make<VkCommandPoolCreateInfo>();
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    create_info.queueFamilyIndex = m_device->getRenderQueueFamilyIndexFromQueu(m_vk_queue);
    m_queue_family_index = create_info.queueFamilyIndex;

    if (vkCreateCommandPool(*m_device, &create_info, nullptr, &m_vk_cmd_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

CommandBufferPool::CommandBufferPool(const CommandBufferPool& other) : m_vk_queue(other.m_vk_queue), m_device(other.m_device) {
    auto create_info = make<VkCommandPoolCreateInfo>();
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    create_info.queueFamilyIndex = m_device->getRenderQueueFamilyIndexFromQueu(m_vk_queue);
    if (vkCreateCommandPool(*m_device, &create_info, nullptr, &m_vk_cmd_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

CommandBufferPool::CommandBufferPool(CommandBufferPool&& other) {
    m_vk_queue = other.m_vk_queue;
    m_device = other.m_device;
    m_vk_cmd_pool = other.m_vk_cmd_pool;
    cmd_buffer_count = other.cmd_buffer_count;

    other.m_vk_cmd_pool = VK_NULL_HANDLE;
}

CommandBufferPool::~CommandBufferPool() {
    if (m_vk_cmd_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(*m_device, m_vk_cmd_pool, nullptr);
    }
}

CommandBufferPool& CommandBufferPool::operator=(const CommandBufferPool& other) {
    if (this != &other) {
        // Destroy the current command pool
        this->~CommandBufferPool();
        m_vk_queue = other.m_vk_queue;
        m_device = other.m_device;
        auto create_info = make<VkCommandPoolCreateInfo>();
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        create_info.queueFamilyIndex = m_device->getRenderQueueFamilyIndexFromQueu(m_vk_queue);
        if (vkCreateCommandPool(*m_device, &create_info, nullptr, &m_vk_cmd_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool");
        }
    }
    return *this;
}

CommandBufferPool& CommandBufferPool::operator=(CommandBufferPool&& other) {
    if (this != &other) {
        this->~CommandBufferPool();
        cmd_buffer_count = std::move(other.cmd_buffer_count);
        m_vk_queue = other.m_vk_queue;
        m_device = other.m_device;
        m_vk_cmd_pool = other.m_vk_cmd_pool;
        other.m_vk_cmd_pool = VK_NULL_HANDLE;
    }
    return *this;
}

std::vector<CommandBuffer> CommandBufferPool::createCommandBuffer(unsigned int p_command_buffer_count) {
    std::vector<CommandBuffer> return_value(p_command_buffer_count);
    std::vector<VkCommandBuffer> command_buffers(p_command_buffer_count);

    auto alloc_info = make<VkCommandBufferAllocateInfo>();
    alloc_info.commandPool = m_vk_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = p_command_buffer_count;
    if (vkAllocateCommandBuffers(*m_device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    for (unsigned int i = 0; i < p_command_buffer_count; i++) {
        return_value[i] = CommandBuffer(m_device, this, command_buffers[i]);
    }
    return return_value;
}

void CommandBufferPool::resetPool() {
    if (vkResetCommandPool(*m_device, m_vk_cmd_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
        throw std::runtime_error("Failed to reset command pool");
    }
}
}  // namespace TTe

// CommandBuffer
namespace TTe {

CommandBuffer::CommandBuffer() {}

CommandBuffer::CommandBuffer(Device* p_device, CommandBufferPool* p_command_buffer_pool, const VkCommandBuffer& p_cmd_buffer)
    : m_cmd_buffer_pool(p_command_buffer_pool), m_vk_cmd_buffer(p_cmd_buffer), m_device(p_device) {}

CommandBuffer::CommandBuffer(CommandBuffer&& other) {
    m_cmd_buffer_pool = other.m_cmd_buffer_pool;
    m_vk_cmd_buffer = other.m_vk_cmd_buffer;
    m_device = other.m_device;
    fini = other.fini;
    m_ressources_to_destroy = std::move(other.m_ressources_to_destroy);
    m_reseted = other.m_reseted;

    other.m_vk_cmd_buffer = VK_NULL_HANDLE;
}

CommandBuffer::~CommandBuffer() {
    if (m_vk_cmd_buffer != VK_NULL_HANDLE) {
        for (auto c : CommandPoolHandler::s_command_pools) {
            if (c.second == this->m_cmd_buffer_pool) {
                vkFreeCommandBuffers(*m_device, m_cmd_buffer_pool->operator()(), 1, &m_vk_cmd_buffer);
            }
            break;
        }
    }
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) {
    if (this != &other) {
        this->~CommandBuffer();

        m_ressources_to_destroy = std::move(other.m_ressources_to_destroy);
        m_cmd_buffer_pool = other.m_cmd_buffer_pool;
        m_vk_cmd_buffer = other.m_vk_cmd_buffer;
        m_device = other.m_device;
        fini = other.fini;
        m_reseted = other.m_reseted;

        other.m_vk_cmd_buffer = VK_NULL_HANDLE;
    }
    return *this;
}

void CommandBuffer::beginCommandBuffer() {
    auto begin_info = make<VkCommandBufferBeginInfo>();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    m_mutex.lock();
    if (vkBeginCommandBuffer(m_vk_cmd_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    for (auto& ressource : m_ressources_to_destroy) {
        if (ressource == this) {
            continue;
        }
        delete ressource;
    }

    m_ressources.clear();
    m_ressources_to_destroy.clear();

    m_reseted = true;
    m_index++;
    fini = false;
    m_mutex.unlock();
}

void CommandBuffer::endCommandBuffer() const {
    if (vkEndCommandBuffer(m_vk_cmd_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
}

void CommandBuffer::submitCommandBuffer(
        std::vector<VkSemaphoreSubmitInfo> p_wait_semaphores,
        std::vector<VkSemaphoreSubmitInfo> p_signal_semaphores,
        Fence* p_vk_fence,
        bool p_wait_for_execution) {
    Semaphore* semaphore = nullptr;

    semaphore = new Semaphore(m_device, VK_SEMAPHORE_TYPE_TIMELINE);
    semaphore->stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    p_signal_semaphores.push_back(semaphore->getSemaphoreSubmitSignalInfo());

    auto cmd_info = make<VkCommandBufferSubmitInfo>();
    cmd_info.commandBuffer = this->m_vk_cmd_buffer;

    auto submit_info = make<VkSubmitInfo2>();
    submit_info.waitSemaphoreInfoCount = p_wait_semaphores.size();
    submit_info.pWaitSemaphoreInfos = p_wait_semaphores.data();
    submit_info.signalSemaphoreInfoCount = p_signal_semaphores.size();
    submit_info.pSignalSemaphoreInfos = p_signal_semaphores.data();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cmd_info;

    VkFence f = (p_vk_fence == nullptr) ? VK_NULL_HANDLE : static_cast<VkFence>(*p_vk_fence);
    m_mutex.lock();

    m_device->getMutexFromQueue(this->m_cmd_buffer_pool->queue()).lock();
    if (vkQueueSubmit2(this->m_cmd_buffer_pool->queue(), 1, &submit_info, f) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit commandBuffer");
    }
    m_device->getMutexFromQueue(this->m_cmd_buffer_pool->queue()).unlock();

    m_reseted = false;
    m_mutex.unlock();
    if (p_wait_for_execution) {
        waitAndDestroy(this, semaphore, m_index);
    } else {
        std::thread(CommandBuffer::waitAndDestroy, this, semaphore, m_index).detach();
    }
}

void CommandBuffer::waitAndDestroy(CommandBuffer* p_cmd_buffer, Semaphore* p_semaphore, uint32_t p_index) {
    p_semaphore->waitTimeLineSemaphore(1);
    p_cmd_buffer->m_mutex.lock();
    if (!p_cmd_buffer->m_reseted && (p_cmd_buffer->m_index == p_index)) {
        // vkResetCommandBuffer(p_cmd_buffer->m_vk_cmd_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        p_cmd_buffer->m_reseted = true;
    }

    p_cmd_buffer->fini = true;
    bool auto_cmd_buffer_destroy = false;

    for (auto& ressource : p_cmd_buffer->m_ressources_to_destroy) {
        if (ressource == p_cmd_buffer) {
            auto_cmd_buffer_destroy = true;
            continue;
        }
        delete ressource;
    }

    p_cmd_buffer->m_ressources.clear();
    p_cmd_buffer->m_ressources_to_destroy.clear();

    delete p_semaphore;
    if (auto_cmd_buffer_destroy) {
        delete p_cmd_buffer;
    }
    p_cmd_buffer->m_mutex.unlock();
}

void CommandBuffer::addRessourceToDestroy(CmdBufferRessource* ressource) { m_ressources_to_destroy.push_back(ressource); }

void CommandBuffer::addRessource(CmdBufferRessource& p_ressource) { m_ressources.push_back(p_ressource); };

}  // namespace TTe