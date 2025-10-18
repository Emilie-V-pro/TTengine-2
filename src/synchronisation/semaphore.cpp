#include "semaphore.hpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <vector>

#include "structs_vk.hpp"
#include "volk.h"

namespace TTe {

Semaphore::Semaphore(Device* p_device, VkSemaphoreType p_vk_semaphore_type) : m_vk_semaphore_type(p_vk_semaphore_type), m_device(p_device) {
    auto type_create_info = make<VkSemaphoreTypeCreateInfo>();
    type_create_info.semaphoreType = m_vk_semaphore_type;
    type_create_info.initialValue = 0;

    auto create_info = make<VkSemaphoreCreateInfo>();
    create_info.pNext = &type_create_info;

    if (vkCreateSemaphore(*m_device, &create_info, nullptr, &m_vk_semaphore) != VK_SUCCESS) {
        std::runtime_error("Failed to create Semaphore");
    }
}

Semaphore::~Semaphore() {
    if (m_vk_semaphore != VK_NULL_HANDLE) vkDestroySemaphore(*m_device, m_vk_semaphore, nullptr);
}

Semaphore::Semaphore(Semaphore&& other) : m_vk_semaphore_type(other.m_vk_semaphore_type), m_device(other.m_device) {
    m_vk_semaphore = other.m_vk_semaphore;
    m_timeline_value = other.m_timeline_value;
    other.m_vk_semaphore = VK_NULL_HANDLE;
}

Semaphore& Semaphore::operator=(Semaphore&& other) {
    if (this != &other) {
        this->~Semaphore();
        m_vk_semaphore = other.m_vk_semaphore;
        m_timeline_value = other.m_timeline_value;
        m_vk_semaphore_type = other.m_vk_semaphore_type;
        m_device = other.m_device;
        
        
        other.m_vk_semaphore = VK_NULL_HANDLE;
    }
    return *this;
}

uint64_t Semaphore::getTimeLineSemaphoreCountValue() const {
    uint64_t create_info;
    assert(m_vk_semaphore_type != VK_SEMAPHORE_TYPE_TIMELINE && "the Semaphore must be a TIMELINE Semaphore to use this function");
    if (vkGetSemaphoreCounterValue(*m_device, m_vk_semaphore, &create_info) != VK_SUCCESS) {
        std::runtime_error("Failed to get Semaphore count value");
    }
    return create_info;
}

VkSemaphoreSubmitInfo Semaphore::getSemaphoreSubmitSignalInfo() {
    auto submit_info = make<VkSemaphoreSubmitInfo>();
    submit_info.semaphore = m_vk_semaphore;
    submit_info.stageMask = stage;
    submit_info.value = m_timeline_value+1;
    m_timeline_value++;
    return submit_info;
}

VkSemaphoreSubmitInfo Semaphore::getSemaphoreSubmitWaittInfo() const {
    auto submit_info = make<VkSemaphoreSubmitInfo>();
    submit_info.semaphore = m_vk_semaphore;
    submit_info.stageMask = stage;
    submit_info.value = m_timeline_value;
    return submit_info;
}

VkResult Semaphore::waitTimeLineSemaphore(uint64_t p_wait_value) const { return waitTimeLineSemaphores(m_device, this, &p_wait_value, 1, false); }

void Semaphore::signalTimeLineSemaphore(uint64_t p_signal_value) const {
    assert(m_vk_semaphore_type != VK_SEMAPHORE_TYPE_TIMELINE && "the Semaphore must be a TIMELINE Semaphore to use this function");
    auto signal_info = make<VkSemaphoreSignalInfo>();
    signal_info.semaphore = m_vk_semaphore;
    signal_info.value = p_signal_value;
    if (vkSignalSemaphore(*m_device, &signal_info) != VK_SUCCESS) {
        std::runtime_error("Failed to signal Timeline Semaphore");
    }
}

VkResult Semaphore::waitTimeLineSemaphores(
    const Device* p_device, const Semaphore * p_semaphores, const uint64_t * p_wait_values, uint32_t p_semaphore_count, bool p_wait_for_first_semaphore) {
    std::vector<VkSemaphore> semaphores_list(p_semaphore_count);

    for (unsigned int i = 0; i < p_semaphore_count; i++) semaphores_list[i] = p_semaphores[i].m_vk_semaphore;

    auto waitInfo = make<VkSemaphoreWaitInfo>();
    waitInfo.semaphoreCount = p_semaphore_count;
    waitInfo.pSemaphores = semaphores_list.data();
    waitInfo.pValues = p_wait_values;
    waitInfo.flags = (p_wait_for_first_semaphore) ? VK_SEMAPHORE_WAIT_ANY_BIT : 0;

    VkResult r = vkWaitSemaphores(*p_device, &waitInfo, std::numeric_limits<uint64_t>::max());

    if (!(r & VK_SUCCESS & VK_TIMEOUT)) {
        std::runtime_error("Failed to wait Timeline Semaphore");
    }
    return r;
}
}  // namespace TTe