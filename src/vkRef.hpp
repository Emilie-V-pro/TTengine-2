#pragma once

#include <mutex>
namespace TTe {
class VkRef {
   public:
    VkRef() {
        mutex.lock();
        refCount = new int(1);
        mutex.unlock();
    }

    ~VkRef() {
        mutex.lock();
        *refCount -= 1;
        if (*refCount == 0) {
            delete refCount;
        }
        mutex.unlock();
    }

    VkRef(VkRef &other) {
        mutex.lock();
        other.mutex.lock();
        refCount = other.refCount;
        *refCount += 1;
        other.mutex.lock();
        mutex.unlock();
    }

    VkRef &operator=(VkRef &other) {
        if (this != &other) {
            mutex.lock();
            other.mutex.lock();
            *refCount -= 1;
            refCount = other.refCount;
            *refCount += 1;
            other.mutex.unlock();
            mutex.unlock();
        }
        return *this;
    }

    VkRef(VkRef &&other) {
        mutex.lock();
        other.mutex.lock();
        refCount = other.refCount;
        other.refCount = nullptr;
        other.mutex.unlock();
        mutex.unlock();
    }

    VkRef &operator=(VkRef &&other) {
        if (this != &other) {
            mutex.lock();
            other.mutex.lock();
            *refCount -= 1;
            refCount = other.refCount;
            other.refCount = nullptr;
            other.mutex.unlock();
            mutex.unlock();
        }
        return *this;
    }

    int *getRefCount() {
        return refCount;
    }

    private:

    std::mutex mutex;
    int *refCount = 0;
};
}  // namespace TTe
