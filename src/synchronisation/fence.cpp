
#include "fence.hpp"

#include <limits>
#include <mutex>
#include <vector>

#include "structs_vk.hpp"
#include "volk.h"

namespace TTe {

Fence::Fence(const Device *p_device, bool p_signaled) : m_device(p_device) {
    auto create_info = make<VkFenceCreateInfo>();
    if (p_signaled) {
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    if (vkCreateFence(*m_device, &create_info, nullptr, &m_vk_fence)) {
        std::runtime_error("Failed to create Fence");
    }
}

Fence::Fence(Fence &&other) : m_vk_fence(other.m_vk_fence), m_device(other.m_device){ other.m_vk_fence = VK_NULL_HANDLE; }

Fence &Fence::operator=(Fence &&other) {
    if (this != &other) {
        this->~Fence();
        this->m_vk_fence = other.m_vk_fence;
        other.m_vk_fence = VK_NULL_HANDLE;
    }
    return *this;
}

Fence::~Fence() {
    if (m_vk_fence != VK_NULL_HANDLE) {
        vkDestroyFence(*m_device, m_vk_fence, nullptr);
    }
}

VkResult Fence::getFenceStatus() {
    VkResult return_value;
    return_value = vkGetFenceStatus(*m_device, m_vk_fence);
    if (return_value == VK_ERROR_DEVICE_LOST) {
        std::runtime_error("Failed to get fence status");
    }
    return return_value;
}

void Fence::resetFence() {
    if (vkResetFences(*m_device, 1, &m_vk_fence) != VK_SUCCESS) {
        std::runtime_error("Failed to reset fence");
    }
}

VkResult Fence::waitForFence() {
    
    VkResult return_value = vkWaitForFences(*m_device, 1, &this->m_vk_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    
    return return_value;
}

VkResult Fence::waitForFences(const Device *p_device, const std::vector<Fence *> &p_fences, bool p_wait_all_fence, int *p_first_fence_signaled) {
    if (p_first_fence_signaled != nullptr) *p_first_fence_signaled = -1;
    std::vector<VkFence> fencesList;
    for (unsigned int i = 0; i < p_fences.size(); i++){
        // if(fences[i]->mutex.try_lock()){
            fencesList.push_back(*p_fences[i]);
        // }
    }
     

    VkBool32 waitAll = (p_wait_all_fence) ? VK_TRUE : VK_FALSE;

    VkResult returnValue = vkWaitForFences(*p_device, fencesList.size(), fencesList.data(), waitAll, std::numeric_limits<uint64_t>::max());

    if (returnValue != VK_SUCCESS && returnValue != VK_TIMEOUT) {
        std::runtime_error("Failed to wait Fences");
    }
    if ((returnValue == VK_SUCCESS) && !p_wait_all_fence && p_first_fence_signaled != nullptr) {
        int i = 0;
        for (auto &fence : p_fences) {
            if (fence->getFenceStatus() == VK_SUCCESS ) {
                *p_first_fence_signaled = i;
                break;
            }
            i++;
        }
    }

    return returnValue;
}
}  // namespace TTe