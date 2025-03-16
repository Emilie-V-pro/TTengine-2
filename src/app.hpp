#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>

#include "Iapp.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "GPU_data/image.hpp"
#include "dynamic_renderpass.hpp"
#include "movement_controller.hpp"
#include "scene/scene.hpp"
#include "sceneV2/scene.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "swapchain.hpp"
#include "synchronisation/semaphore.hpp"
#include "circular_queue.hpp"
#include "utils.hpp"
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
    void renderFrame(float deltatTime, CommandBuffer& cmdBuffer, uint32_t curentFrameIndex);

   private:
  
    SwapChain* swapchain;
    
    DynamicRenderPass renderPass;
    MovementController movementController;

    Scene scene;
    Scene2 scene2;

    float time = 0.0f;
};
}  // namespace TTe