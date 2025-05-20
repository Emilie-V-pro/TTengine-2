
#include "engine.hpp"
#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "GPU_data/image.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"

#define IMGUI_IMPL_VULKAN_USE_VOLK
#include "imgui_impl_vulkan.h"
#include "synchronisation/semaphore.hpp"
namespace TTe {

Engine::~Engine() {
    vkDeviceWaitIdle(device);
    delete app;
    Image::destroySamplers(&device);
    // CommandPoolHandler::destroyCommandPools();
}

void Engine::init() {
    Image::createsamplers(&device);
    app->init(&device, &swapChain, &window);

    // device wait idle
    vkDeviceWaitIdle(device);

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        waitToPresentSemaphores.emplace_back(&device, VK_SEMAPHORE_TYPE_BINARY);
        renderCommandBuffers[i] = std::move(commandBufferPool->createCommandBuffer(1)[0]);
    }
    updateCommandBuffer = std::move(CommandPoolHandler::getCommandPool(&device, device.getComputeQueue())->createCommandBuffer(1)[0]);

    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool);

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkFormat format = swapChain.getSwapChainImage(0).getFormat();
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = device.getInstance();
    init_info.PhysicalDevice = device.getVkbDevice().physical_device;
    init_info.Device = device;
    init_info.Queue = device.getRenderQueue();
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.UseDynamicRendering = true;
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &format;

    auto test = vkGetInstanceProcAddr(device.getInstance(), "vkCmdBeginRendering");

    renderPass = DynamicRenderPass(&device, {1280, 720}, {}, swapChain.getswapChainImages().size(), depthAndStencil::NONE, &swapChain, nullptr);
        renderPass.setClearEnable(false);
    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();
}
void Engine::run() {
    // create thread for update loop

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
    auto extent = window.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.getExtent();
        glfwWaitEvents();
    }
    resizeMutex.lock();
    vkDeviceWaitIdle(device);
    swapChain.recreateSwapchain(extent);
    renderPass.resize(extent);
    renderPass.setClearEnable(false);
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
    while (!engine.shouldClose) {
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
        ImGui::ShowDemoWindow();
   
      
        engine.renderCommandBuffers[engine.renderIndex].beginCommandBuffer();
      
 
        engine.app->renderFrame(deltatTime, engine.renderCommandBuffers[engine.renderIndex], engine.currentSwapchainImage, engine.renderIndex);
    
      
        engine.renderPass.beginRenderPass(engine.renderCommandBuffers[engine.renderIndex], engine.currentSwapchainImage);
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), engine.renderCommandBuffers[engine.renderIndex]);
        engine.renderPass.endRenderPass(engine.renderCommandBuffers[engine.renderIndex]);
  
      
  
        engine.swapChain.getSwapChainImage(engine.currentSwapchainImage)
            .transitionImageLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &engine.renderCommandBuffers[engine.renderIndex]);


       

        engine.renderCommandBuffers[engine.renderIndex].endCommandBuffer();
  
     
  
        engine.renderCommandBuffers[engine.renderIndex].submitCommandBuffer(
            {aquireFrameSemaphore->getSemaphoreSubmitWaittInfo()},
            {engine.waitToPresentSemaphores[engine.renderIndex].getSemaphoreSubmitSignalInfo()}, fence, false);
     
   
        engine.endAndPresentFrame(&engine.waitToPresentSemaphores[engine.renderIndex]);
  
        float timeEndAndPresentFrame = std::chrono::duration<float, std::chrono::seconds::period>(cE - cS).count();

        // std::cout << "timeStartFrame : " << timeStartFrame << " ImGuiTime : " << ImGuiTime << " timeBeginCommandBuffer : " << timeBeginCommandBuffer << " timeRenderFrame : " << timeRenderFrame << " timeEndRenderPass : " << timeEndRenderPass << " timeTransitionImageLayout : " << timeTransitionImageLayout << " timeEndCommandBuffer : " << timeEndCommandBuffer << " timeSubmitCommandBuffer : " << timeSubmitCommandBuffer << " timeEndAndPresentFrame : " << timeEndAndPresentFrame << "\n";
    }
}

void Engine::updateLoop(Engine &engine) {
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
            engine.app->update(deltatTime, engine.updateCommandBuffer, engine.window);
            engine.resizeMutex.unlock();
        }
    }
}

}  // namespace TTe