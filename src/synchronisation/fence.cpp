
#include "fence.hpp"

#include <limits>
#include <mutex>
#include <vector>

#include "structs_vk.hpp"
#include "volk.h"

namespace TTe {

Fence::Fence(const Device *device, bool signaled) : device(device) {
    auto createInfo = make<VkFenceCreateInfo>();
    if (signaled) {
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    if (vkCreateFence(*device, &createInfo, nullptr, &vk_Fence)) {
        std::runtime_error("Failed to create Fence");
    }
}

Fence::Fence(Fence &&other) : vk_Fence(other.vk_Fence), device(other.device){ other.vk_Fence = VK_NULL_HANDLE; }

Fence &Fence::operator=(Fence &&other) {
    if (this != &other) {
        this->~Fence();
        this->vk_Fence = other.vk_Fence;
        other.vk_Fence = VK_NULL_HANDLE;
    }
    return *this;
}

Fence::~Fence() {
    if (vk_Fence != VK_NULL_HANDLE) {
        vkDestroyFence(*device, vk_Fence, nullptr);
    }
}

VkResult Fence::getFenceStatus() {
    VkResult returnValue;
    returnValue = vkGetFenceStatus(*device, vk_Fence);
    if (returnValue == VK_ERROR_DEVICE_LOST) {
        std::runtime_error("Failed to get fence status");
    }
    return returnValue;
}

void Fence::resetFence() {
    if (vkResetFences(*device, 1, &vk_Fence) != VK_SUCCESS) {
        std::runtime_error("Failed to reset fence");
    }
}

VkResult Fence::waitForFence() {
    
    VkResult returnValue = vkWaitForFences(*device, 1, &this->vk_Fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    
    return returnValue;
}

VkResult Fence::waitForFences(const Device *device, const std::vector<Fence *> &fences, bool waitAllFence, int *fenceSignaled) {
    if (fenceSignaled != nullptr) *fenceSignaled = -1;
    std::vector<VkFence> fencesList;
    for (unsigned int i = 0; i < fences.size(); i++){
        // if(fences[i]->mutex.try_lock()){
            fencesList.push_back(*fences[i]);
        // }
    }
     

    VkBool32 waitAll = (waitAllFence) ? VK_TRUE : VK_FALSE;

    VkResult returnValue = vkWaitForFences(*device, fencesList.size(), fencesList.data(), waitAll, std::numeric_limits<uint64_t>::max());

    if (returnValue != VK_SUCCESS && returnValue != VK_TIMEOUT) {
        std::runtime_error("Failed to wait Fences");
    }
    if ((returnValue == VK_SUCCESS) && !waitAllFence && fenceSignaled != nullptr) {
        int i = 0;
        for (auto &fence : fences) {
            if (fence->getFenceStatus() == VK_SUCCESS ) {
                *fenceSignaled = i;
                break;
            }
            i++;
        }
    }

    return returnValue;
}
}  // namespace TTe