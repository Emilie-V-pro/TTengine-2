#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>

#include "Iapp.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "image.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "synchronisation/semaphore.hpp"
#include "utils.hpp"

namespace TTe {
class App : public IApp {
   public:
    ~App() {};
    // set up the application
    void init(Device* device, std::vector<Image>& swapchainImages);
    void resize(int width, int height, std::vector<Image>& swapchainImages);

    // update the application
    void update(float deltaTime, CommandBuffer& cmdBuffer);
    void renderFrame(float deltatTime, CommandBuffer& cmdBuffer, uint32_t curentFrameIndex);

   private:
    std::mutex testMutex;
    ComputePipeline computePipeline;
    Image image;
    std::shared_ptr<Image> renderedImage;
    DescriptorSet descriptorSet;
    std::array<Semaphore, MAX_FRAMES_IN_FLIGHT> imageRenderdSemaphores;
    std::vector<Image>* swapchainImages;
};
}  // namespace TTe