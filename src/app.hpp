#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Iapp.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "movement_controller.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/portal_controller.hpp"
#include "sceneV2/scene.hpp"
#include "swapchain.hpp"
#include "circular_queue.hpp"
#include "window.hpp"

namespace TTe {
class App : public IApp {
   public:
    App() {};
    ~App() {};
    // set up the application
    void init(Device* device, SwapChain* swapchain, Window* window);
    void resize(int width, int height);

    // update the application
    void update(float deltaTime, CommandBuffer& cmdBuffer, Window& windowObj);
    void renderFrame(float deltatTime, CommandBuffer& cmdBuffer, uint32_t curentFrameIndex, uint32_t render_index);

   private:

    uint32_t sphere_hit_id = 0;
    uint32_t portal_id = 0;
  
    SwapChain* swapchain;
    
    DynamicRenderPass renderPass;
    MovementController movementController;

    std::vector<DynamicRenderPass> portalARenderPasses;
    std::vector<DynamicRenderPass> portalBRenderPasses;

    std::shared_ptr<SkeletonObj> skeleton;

    
    std::shared_ptr<Scene> scene2;

    float near = 0.1f;
    float x_rot = 0.0f;

    float time = 0.0f;
    uint32_t tick = 0; 
};
}  // namespace TTe