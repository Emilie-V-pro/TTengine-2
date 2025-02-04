
#include "engine.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "synchronisation/semaphore.hpp"
namespace TTe {

Engine::~Engine() {
    delete app;
    vkDeviceWaitIdle(device);
    CommandPoolHandler::cleanUnusedPools();
    
}

void Engine::init() {
    app->init(&device, swapChain.getswapChainImages());
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        waitToPresentSemaphores.emplace_back(&device, VK_SEMAPHORE_TYPE_BINARY);
        renderCommandBuffers[i] = std::move(commandBufferPool->createCommandBuffer(1)[0]);
    }
    updateCommandBuffer = std::move(CommandPoolHandler::getCommandPool(&device, device.getComputeQueue())->createCommandBuffer(1)[0]);
}
void Engine::run() {
    // create thread for update loop
    std::thread updateThread(&Engine::updateLoop, std::ref(*this));

    std::thread renderThread(&Engine::renderLoop, std::ref(*this));

    updateThread.join();
    renderThread.join();
    
}

bool Engine::startFrame(Semaphore *&aquireFrameSemaphore, Fence *&fence) {
    VkResult result = swapChain.acquireNextImage(currentSwapchainImage, &renderIndex, aquireFrameSemaphore, fence);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        resize();
        return false;
    }
    if (result == VK_TIMEOUT) {
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    return true;
}

void Engine::resize() {
    auto extent = window.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.getExtent();
        glfwWaitEvents();
    }
    resizeMutex.lock();
    vkDeviceWaitIdle(device);
    swapChain.recreateSwapchain(extent);
    app->resize(extent.width, extent.height, swapChain.getswapChainImages());
    resizeMutex.unlock();
}

void Engine::endAndPresentFrame(Semaphore *waitRenderSemaphore) {
    auto result = swapChain.presentFrame(currentSwapchainImage, waitRenderSemaphore);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
        window.resetWindowResizedFlag();
        resize();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void Engine::renderLoop(Engine &engine) {
    std::cout << "render loop avec le thread : " << std::this_thread::get_id() << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    while (!engine.shouldClose) {
        auto newTime = std::chrono::high_resolution_clock::now();

        float deltatTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - start).count();
        start = newTime;
        Semaphore *aquireFrameSemaphore = nullptr;
        Fence *fence = nullptr;
        if (!engine.startFrame(aquireFrameSemaphore, fence)) {
            continue;
        }
        engine.renderCommandBuffers[engine.renderIndex].beginCommandBuffer();

        engine.app->renderFrame(deltatTime, engine.renderCommandBuffers[engine.renderIndex], engine.currentSwapchainImage);

        engine.swapChain.getSwapChainImage(engine.currentSwapchainImage)
            .transitionImageLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &engine.renderCommandBuffers[engine.renderIndex]);

        engine.renderCommandBuffers[engine.renderIndex].endCommandBuffer();

        engine.renderCommandBuffers[engine.renderIndex].submitCommandBuffer(
            {aquireFrameSemaphore->getSemaphoreSubmitWaittInfo()},
            {engine.waitToPresentSemaphores[engine.renderIndex].getSemaphoreSubmitSignalInfo()}, fence, false);

        engine.endAndPresentFrame(&engine.waitToPresentSemaphores[engine.renderIndex]);
    }
}

void Engine::updateLoop(Engine &engine) {
    std::cout << "update loop avec le thread : " << std::this_thread::get_id() << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    uint32_t frameIndex = 0;
    while (!engine.shouldClose) {
        glfwPollEvents();
        if (engine.resizeMutex.try_lock()) {
            frameIndex++;
            auto newTime = std::chrono::high_resolution_clock::now();
            float deltatTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - start).count();
            start = newTime;

            // engine.updateCommandBuffer.beginCommandBuffer();
            // engine.app->update(deltatTime, engine.updateCommandBuffer);
            
            engine.resizeMutex.unlock();
            if (frameIndex == 1000000000) {
                engine.shouldClose = true;
            }
        }
    }
}

}  // namespace TTe