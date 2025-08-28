#pragma once

#include <cstdint>
#include <vector>
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "GPU_data/image.hpp"
#include "dynamic_renderpass.hpp"
#include "swapchain.hpp"
namespace TTe {
class IApp {
   public:
    // set up the application
    virtual ~IApp() = default;
    void virtual init(Device* device, DynamicRenderPass *deferredRenderPass, DynamicRenderPass *shadingRenderPass, Window* window) = 0;
    void virtual resize(int width, int height) = 0;

    // update the application
    void virtual update(float deltaTime, CommandBuffer &cmdBuffer, Window& windowObj) = 0;
    void virtual renderDeferredFrame(float deltatTime, CommandBuffer &cmdBuffer ,uint32_t render_index, uint32_t swapchainIndex) = 0;
    void virtual renderShadedFrame(float deltatTime, CommandBuffer &cmdBuffer,uint32_t render_index, uint32_t swapchainIndex) = 0;
   protected:
   Device *device;
   private:

};
}  // namespace TTe