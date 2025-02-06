#pragma once

#include <cstdint>
#include <vector>
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "GPU_data/image.hpp"
#include "swapchain.hpp"
namespace TTe {
class IApp {
   public:
    // set up the application
    virtual ~IApp() = default;
    void virtual init(Device* device, SwapChain* swapchainImages) = 0;
    void virtual resize(int width, int height) = 0;

    // update the application
    void virtual update(float deltaTime, CommandBuffer &cmdBuffer, Window& windowObj) = 0;
    void virtual renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex) = 0;
   protected:
   Device *device;
   private:

};
}  // namespace TTe