#pragma once

#include <cstdint>
#include <string>
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
namespace TTe {
class IApp {
   public:
    // set up the application
    virtual ~IApp() = default;
    void virtual init(Device* p_device, DynamicRenderPass *p_deferred_renderpass, DynamicRenderPass *p_shading_renderpass, Window* p_window) = 0;
    void virtual resize(int p_width, int p_height) = 0;

    // update the application
    void virtual update(float p_delta_time, CommandBuffer &p_cmd_buffer, Window& p_window_obj) = 0;
    void virtual renderDeferredFrame(float p_delta_time, CommandBuffer &p_cmd_buffer ,uint32_t p_render_index, uint32_t p_swapchain_index) = 0;
    void virtual renderShadedFrame(float p_delta_time, CommandBuffer &p_cmd_buffer,uint32_t p_render_index, uint32_t p_swapchain_index) = 0;
    std::string name;
   protected:
   Device *device;
   private:

};
}  // namespace TTe