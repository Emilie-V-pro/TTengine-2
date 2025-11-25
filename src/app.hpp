#pragma once

#include <array>
#include <cstdint>
#include <glm/fwd.hpp>
#include <mutex>
#include <vector>

#include "Iapp.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/main_controller.hpp"
#include "sceneV2/scene.hpp"
#include "utils.hpp"
#include "window.hpp"

namespace TTe {

#define MAX_LIGHTS 64

class App : public IApp {
   public:
    App() = default;
    ~App();
    // set up the application
    void init(Device* p_device, DynamicRenderPass *p_deferred_renderpass, DynamicRenderPass *p_shading_renderpass, Window* p_window);
    void resize(int p_width, int p_height);

    // update the application
    void update(float p_delta_time, CommandBuffer &p_cmd_buffer, Window& p_window_obj) ;
    void renderDeferredFrame(float p_deltat_time, CommandBuffer &p_cmd_buffer ,uint32_t p_render_index, uint32_t p_swapchain_index) ;
    void renderShadedFrame(float p_deltat_time, CommandBuffer &p_cmd_buffer, uint32_t p_render_index, uint32_t p_swapchain_index) ;
   private:

   std::vector<glm::vec3> m_light_accelerations;
   std::vector<glm::vec3> m_light_speeds;

   DynamicRenderPass *m_deferred_renderpass = nullptr;
   DynamicRenderPass *m_shading_renderpass = nullptr;
   Scene *s;
   MainController m_movement_controller;
   std::array<bool, MAX_FRAMES_IN_FLIGHT> update_culling = {true, true};
   std::mutex m;


  


};
}  // namespace TTe