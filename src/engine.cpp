
#include "engine.hpp"

#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "GPU_data/image.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "utils.hpp"

#define IMGUI_IMPL_VULKAN_USE_VOLK
#include "imgui_impl_vulkan.h"
#include "synchronisation/semaphore.hpp"
namespace TTe {

Engine::~Engine() {
    vkDeviceWaitIdle(device);
    delete app;
    Image::destroySamplers(&device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    CommandPoolHandler::destroyCommandPools();
}

void Engine::init() {
    Image::createsamplers(&device);
    
    

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        defferedRenderCommandBuffers[i] = std::move(commandBufferPool->createCommandBuffer(1)[0]);
        shadingRenderCommandBuffers[i] = std::move(commandBufferPool->createCommandBuffer(1)[0]);
        deferredRenderSemaphores[i] = Semaphore(&device, VK_SEMAPHORE_TYPE_BINARY);
        deferredRenderSemaphores[i].stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    for (unsigned int i = 0; i < swapChain.getswapChainImages().size(); i++) {
        waitToPresentSemaphores.emplace_back(&device, VK_SEMAPHORE_TYPE_BINARY);
    }
    updateCommandBuffer = std::move(CommandPoolHandler::getCommandPool(&device, device.getComputeQueue())->createCommandBuffer(1)[0]);
    
    
        

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(window, true);
    
    VkFormat format = swapChain.getSwapChainImage(0).getFormat();
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = device.getInstance();
    init_info.PhysicalDevice = device.getVkbDevice().physical_device;
    init_info.Device = device;
    init_info.Queue = device.getRenderQueue();
    // init_info.DescriptorPool = imguiPool;
    init_info.DescriptorPoolSize = 1000;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.UseDynamicRendering = true;
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &format;
    
    auto test = vkGetInstanceProcAddr(device.getInstance(), "vkCmdBeginRendering");
    
    ImGui_ImplVulkan_Init(&init_info);
    
    ImGui_ImplVulkan_CreateFontsTexture();
   

    // create renderPass

    deferredRenderPass = DynamicRenderPass(
        &device, window.getExtent(), {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM}, swapChain.getswapChainImages().size(), DEPTH, nullptr, nullptr);

 
    shadingRenderPass = DynamicRenderPass(
        &device, window.getExtent(), {}, swapChain.getswapChainImages().size(), depthAndStencil::DEPTH, &swapChain, deferredRenderPass.getDepthAndStencilPtr());

    shadingRenderPass.setClearEnable(false);
    app->init(&device, &deferredRenderPass, &shadingRenderPass, &window);
 

    swapChain.getSwapChainImage(0).transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL);

    // device wait idle
    vkDeviceWaitIdle(device);
}
void Engine::run() {
    // create thread for update loop

    resize();

    std::cout << "DÉBUT RENDU" << std::endl;
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
    auto extent = window.getExtentGLFW();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.getExtent();
        glfwWaitEvents();
    }
    resizeMutex.lock();
    vkDeviceWaitIdle(device);
    swapChain.recreateSwapchain(extent);
    deferredRenderPass.resize(extent);
    shadingRenderPass.resize(extent);
    shadingRenderPass.setClearEnable(false);
    app->resize(extent.width, extent.height);
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
    auto start = std::chrono::high_resolution_clock::now();

    while (!engine.window.shouldClose()) {
        auto newTime = std::chrono::high_resolution_clock::now();

        float deltatTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - start).count();
        start = newTime;
        Semaphore *aquireFrameSemaphore = nullptr;
        Fence *fence = nullptr;

        auto cS = std::chrono::high_resolution_clock::now();
        if (!engine.startFrame(aquireFrameSemaphore, fence)) {
            continue;
        }
        auto cE = std::chrono::high_resolution_clock::now();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // DEFERRED RENDERING

        engine.defferedRenderCommandBuffers[engine.renderIndex].beginCommandBuffer();

        engine.app->renderDeferredFrame(
            deltatTime, engine.defferedRenderCommandBuffers[engine.renderIndex], engine.renderIndex, engine.currentSwapchainImage);

        engine.defferedRenderCommandBuffers[engine.renderIndex].endCommandBuffer();

        engine.defferedRenderCommandBuffers[engine.renderIndex].submitCommandBuffer(
            {}, {engine.deferredRenderSemaphores[engine.renderIndex].getSemaphoreSubmitSignalInfo()});

        // SHADING RENDERING
        engine.shadingRenderCommandBuffers[engine.renderIndex].beginCommandBuffer();

        engine.swapChain.getSwapChainImage(engine.currentSwapchainImage)
            .transitionImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, &engine.shadingRenderCommandBuffers[engine.renderIndex]);


        engine.app->renderShadedFrame(
            deltatTime, engine.shadingRenderCommandBuffers[engine.renderIndex], engine.renderIndex, engine.currentSwapchainImage);

        // UI RENDERING

        engine.shadingRenderPass.beginRenderPass(engine.shadingRenderCommandBuffers[engine.renderIndex], engine.currentSwapchainImage);
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), engine.shadingRenderCommandBuffers[engine.renderIndex]);
        engine.shadingRenderPass.endRenderPass(engine.shadingRenderCommandBuffers[engine.renderIndex]);

        engine.swapChain.getSwapChainImage(engine.currentSwapchainImage)
            .transitionImageLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &engine.shadingRenderCommandBuffers[engine.renderIndex]);

        engine.shadingRenderCommandBuffers[engine.renderIndex].endCommandBuffer();

        engine.shadingRenderCommandBuffers[engine.renderIndex].submitCommandBuffer(
            {aquireFrameSemaphore->getSemaphoreSubmitWaittInfo(), engine.deferredRenderSemaphores[engine.renderIndex].getSemaphoreSubmitSignalInfo()},
            {engine.waitToPresentSemaphores[engine.currentSwapchainImage].getSemaphoreSubmitSignalInfo()}, fence, false);

        engine.endAndPresentFrame(&engine.waitToPresentSemaphores[engine.currentSwapchainImage]);

        float timeEndAndPresentFrame = std::chrono::duration<float, std::chrono::seconds::period>(cE - cS).count();

    }
}

void Engine::updateLoop(Engine &engine) {
    auto start = std::chrono::high_resolution_clock::now();
    uint32_t frameIndex = 0;
    while (!engine.window.shouldClose()) {
        glfwPollEvents();
        if (engine.resizeMutex.try_lock()) {
            frameIndex++;
            auto newTime = std::chrono::high_resolution_clock::now();
            float deltatTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - start).count();
            start = newTime;

            // engine.updateCommandBuffer.beginCommandBuffer();
            engine.app->update(deltatTime, engine.updateCommandBuffer, engine.window);
            engine.resizeMutex.unlock();
        }
    }
}

}  // namespace TTe