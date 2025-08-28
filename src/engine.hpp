#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "Iapp.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "swapchain.hpp"
#include "synchronisation/semaphore.hpp"
#include "utils.hpp"
#include "window.hpp"
namespace TTe {

class Engine {
   public:
    // Constructor
    Engine(IApp *app) : app(app) {};

    // Destructor
    ~Engine();

    // Remove Copy and move constructors
    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(Engine &&) = delete;
    Engine &operator=(Engine &&) = delete;

    void init();
    void run();

   private:

   
    void resize();

    bool startFrame(Semaphore *&aquireFrameSemaphore, Fence *&fence);
    void endAndPresentFrame(Semaphore *waitRenderSemaphore);

    static void renderLoop(Engine &engine);
    static void updateLoop(Engine &engine);

    uint32_t currentSwapchainImage = 0;
    int renderIndex = 0;

    bool shouldClose = false;

    std::mutex resizeMutex;

    Window window{1280, 720, "mon napli"};
    Device device{window};
    SwapChain swapChain{&device, window.getExtent(), vkb::SwapchainBuilder::BufferMode::DOUBLE_BUFFERING};

    IApp *app;
    
    std::vector<Semaphore> waitToPresentSemaphores;
    
    CommandBufferPool *commandBufferPool = CommandPoolHandler::getCommandPool(&device, device.getRenderQueue());
    
    CommandBuffer updateCommandBuffer;
    
    // RenderData
    std::array<Semaphore, MAX_FRAMES_IN_FLIGHT>  deferredRenderSemaphores;
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> defferedRenderCommandBuffers;
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> shadingRenderCommandBuffers;
    
    DynamicRenderPass deferredRenderPass;
    DynamicRenderPass shadingRenderPass;

    Buffer indirectDrawBuffer;
    Buffer indirectDrawBufferCount;



    
};
}  // namespace TTe