#include "semaphore.hpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <vector>

#include "structs_vk.hpp"
#include "volk.h"

namespace TTe {

Semaphore::Semaphore(Device* device, VkSemaphoreType vkSemaphoreType) : vkSemaphoreType(vkSemaphoreType), device(device) {
    auto typeCreateInfo = make<VkSemaphoreTypeCreateInfo>();
    typeCreateInfo.semaphoreType = vkSemaphoreType;
    typeCreateInfo.initialValue = 0;

    auto createInfo = make<VkSemaphoreCreateInfo>();
    createInfo.pNext = &typeCreateInfo;

    if (vkCreateSemaphore(*device, &createInfo, nullptr, &vksemaphore) != VK_SUCCESS) {
        std::runtime_error("Failed to create Semaphore");
    }
}

Semaphore::~Semaphore() {
    if (vksemaphore != VK_NULL_HANDLE) vkDestroySemaphore(*device, vksemaphore, nullptr);
}

Semaphore::Semaphore(Semaphore&& semaphore) : vkSemaphoreType(semaphore.vkSemaphoreType), device(semaphore.device) {
    vksemaphore = semaphore.vksemaphore;
    timelineValue = semaphore.timelineValue;
    semaphore.vksemaphore = VK_NULL_HANDLE;
}

Semaphore& Semaphore::operator=(Semaphore&& semaphore) {
    if (this != &semaphore) {
        this->~Semaphore();
        vksemaphore = semaphore.vksemaphore;
        timelineValue = semaphore.timelineValue;
        semaphore.vksemaphore = VK_NULL_HANDLE;
    }
    return *this;
}

uint64_t Semaphore::getTimeLineSemaphoreCountValue() const {
    uint64_t returnValue;
    assert(vkSemaphoreType != VK_SEMAPHORE_TYPE_TIMELINE && "the Semaphore must be a TIMELINE Semaphore to use this function");
    if (vkGetSemaphoreCounterValue(*device, vksemaphore, &returnValue) != VK_SUCCESS) {
        std::runtime_error("Failed to get Semaphore count value");
    }
    return returnValue;
}

VkSemaphoreSubmitInfo Semaphore::getSemaphoreSubmitSignalInfo() {
    auto submitInfo = make<VkSemaphoreSubmitInfo>();
    submitInfo.semaphore = vksemaphore;
    submitInfo.stageMask = signalStage;
    submitInfo.value = timelineValue+1;
    timelineValue++;
    return submitInfo;
}

VkSemaphoreSubmitInfo Semaphore::getSemaphoreSubmitWaittInfo() const {
    auto submitInfo = make<VkSemaphoreSubmitInfo>();
    submitInfo.semaphore = vksemaphore;
    submitInfo.stageMask = waitStage;
    submitInfo.value = timelineValue;
    return submitInfo;
}

VkResult Semaphore::waitTimeLineSemaphore(uint64_t waitValue) const { return waitTimeLineSemaphores(device, {this}, {waitValue}, false); }

void Semaphore::signalTimeLineSemaphore(uint64_t signalValue) const {
    assert(vkSemaphoreType != VK_SEMAPHORE_TYPE_TIMELINE && "the Semaphore must be a TIMELINE Semaphore to use this function");
    auto signalInfo = make<VkSemaphoreSignalInfo>();
    signalInfo.semaphore = vksemaphore;
    signalInfo.value = signalValue;
    if (vkSignalSemaphore(*device, &signalInfo) != VK_SUCCESS) {
        std::runtime_error("Failed to signal Timeline Semaphore");
    }
}

VkResult Semaphore::waitTimeLineSemaphores(
    const Device* device, std::vector<const Semaphore*> semaphores, std::vector<uint64_t> waitValues, bool waitForFirstSemaphore) {
    assert(semaphores.size() == waitValues.size() && "You need to provide exactly one semaphore per waitValue");
    std::vector<VkSemaphore> semaphoresList(semaphores.size());

    for (unsigned int i = 0; i < semaphores.size(); i++) semaphoresList[i] = semaphores[i]->vksemaphore;

    auto waitInfo = make<VkSemaphoreWaitInfo>();
    waitInfo.semaphoreCount = semaphores.size();
    waitInfo.pSemaphores = semaphoresList.data();
    waitInfo.pValues = waitValues.data();
    waitInfo.flags = (waitForFirstSemaphore) ? VK_SEMAPHORE_WAIT_ANY_BIT : 0;

    VkResult r = vkWaitSemaphores(*device, &waitInfo, std::numeric_limits<uint64_t>::max());

    if (!(r & VK_SUCCESS & VK_TIMEOUT)) {
        std::runtime_error("Failed to wait Timeline Semaphore");
    }
    return r;
}
}  // namespace TTe