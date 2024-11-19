#pragma once

#include "device.hpp"
namespace TTe {
class CommandBufferPoolHandler {
   public:
    CommandBufferPoolHandler();
    CommandBufferPoolHandler(const CommandBufferPoolHandler& cmdPoolHandler);
    CommandBufferPoolHandler(const CommandBufferPoolHandler&& cmdPoolHandler);
    CommandBufferPoolHandler(const Device* device, const VkQueue& queue);

    ~CommandBufferPoolHandler();
    CommandBufferPoolHandler &operator=(const CommandBufferPoolHandler& cmdPoolHandler);
    CommandBufferPoolHandler &operator=(const CommandBufferPoolHandler&& cmdPoolHandler);
    


   private:
};

class CommandBuffer {};
}  // namespace TTe