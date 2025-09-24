#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "Iapp.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/main_controller.hpp"
#include "sceneV2/main_controller_col.hpp"
#include "sceneV2/scene.hpp"
#include "window.hpp"

namespace TTe {
class App : public IApp {
   public:
    App() = default;
    ~App();
    // set up the application
    void init(Device* device, DynamicRenderPass *deferredRenderPass, DynamicRenderPass *shadingRenderPass, Window* window);
    void resize(int width, int height);

    // update the application
    void update(float deltaTime, CommandBuffer &cmdBuffer, Window& windowObj) ;
    void renderDeferredFrame(float deltatTime, CommandBuffer &cmdBuffer ,uint32_t render_index, uint32_t swapchainIndex) ;
    void renderShadedFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t render_index, uint32_t swapchainIndex) ;
   private:

   DynamicRenderPass *deferredRenderPass = nullptr;
   DynamicRenderPass *shadingRenderPass = nullptr;
   Scene *s;
   MainControllerCOL movementController;

   std::mutex m;


  


};
}  // namespace TTe