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
#include "scene/scene.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "swapchain.hpp"
#include "synchronisation/semaphore.hpp"
#include "utils.hpp"

namespace TTe {
class App : public IApp {
   public:
    App() {};
    ~App() {};
    // set up the application
    void init(Device* device, SwapChain* swapchain);
    void resize(int width, int height);

    // update the application
    void update(float deltaTime, CommandBuffer& cmdBuffer);
    void renderFrame(float deltatTime, CommandBuffer& cmdBuffer, uint32_t curentFrameIndex);

   private:
    std::mutex testMutex;
    ComputePipeline computePipeline;
    Image image;
    Image* renderedImage;
    DescriptorSet descriptorSet;
    std::array<Semaphore, MAX_FRAMES_IN_FLIGHT> imageRenderdSemaphores;
    SwapChain* swapchain;
    
    DynamicRenderPass renderPass;
    Scene scene;
};
}  // namespace TTe