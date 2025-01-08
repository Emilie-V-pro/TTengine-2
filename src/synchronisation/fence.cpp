
#include "fence.hpp"

#include "volk.h"


#include <limits>
#include <vector>
#include "../structs_vk.hpp"

namespace TTe {

Fence::Fence(const Device *device, bool signaled) : device(device) {
    auto createInfo = make<VkFenceCreateInfo>();
    if (signaled) {
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    if (vkCreateFence(device->device(), &createInfo, nullptr, &vk_Fence)) {
        std::runtime_error("Failed to create Fence");
    }
}

Fence::~Fence() {
    if(vk_Fence != VK_NULL_HANDLE) {
        vkDestroyFence(device->device(), vk_Fence, nullptr);
    }
}


VkResult Fence::getFenceStatus() const {
    VkResult returnValue;
    returnValue = vkGetFenceStatus(device->device(), vk_Fence);
    if (returnValue == VK_ERROR_DEVICE_LOST) {
        std::runtime_error("Failed to get fence status");
    }
    return returnValue;
}

void Fence::resetFence() {
    if (vkResetFences(device->device(), 1, &vk_Fence) != VK_SUCCESS) {
        std::runtime_error("Failed to reset fence");
    }
}

VkResult Fence::waitForFence() { return waitForFences(device, {this}, true, nullptr); }

VkResult Fence::waitForFences(const Device *device, std::vector<Fence*> &fences, bool waitAllFence, size_t *fenceSignaled) {
    if(fenceSignaled!= nullptr) *fenceSignaled = -1;
    std::vector<VkFence> fencesList(fences.size());
    for (int i = 0; i < fences.size(); i++) fencesList[i] = fences[i].vkFence;
    VkBool32 waitAll = (waitAllFence) ? VK_TRUE : VK_FALSE;
    VkResult returnValue =
        vkWaitForFences(device->device(), fencesList.size(), fencesList.data(), waitAll, std::numeric_limits<uint64_t>::max());
    if (returnValue != VK_SUCCESS &&  returnValue != VK_TIMEOUT) {
        std::runtime_error("Failed to wait Fences");
    }
    if ((returnValue == VK_SUCCESS) && !waitAllFence && fenceSignaled != nullptr) {
        int i = 0;
        for (auto &fence : fences) {
            if (fence.getFenceStatus() == VK_SUCCESS) {
                *fenceSignaled = i;
                break;
            }
            i++;
        }
    }

    return returnValue;
}
}  // namespace vk_stage