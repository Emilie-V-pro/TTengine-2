
#include "engine.hpp"



#include <chrono>
#include <iostream>
#include <thread>

#include "GPU_data/image.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "utils.hpp"

#define IMGUI_IMPL_VULKAN_USE_VOLK
#include "imgui_impl_vulkan.h"
#include "synchronisation/semaphore.hpp"
namespace TTe {

Engine::~Engine() {
    vkDeviceWaitIdle(m_device);
    delete m_app;
    Image::destroySamplers(&m_device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    CommandPoolHandler::destroyCommandPools();
}

void Engine::init() {
    Image::createsamplers(&m_device);
    
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_deffered_render_cmd_buffers[i] = std::move(m_cmd_buffer_pool->createCommandBuffer(1)[0]);
        m_shading_render_cmd_buffers[i] = std::move(m_cmd_buffer_pool->createCommandBuffer(1)[0]);
        m_deferred_render_semaphores[i] = Semaphore(&m_device, VK_SEMAPHORE_TYPE_BINARY);
        m_deferred_render_semaphores[i].stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    for (unsigned int i = 0; i < m_swapchain.getswapChainImages().size(); i++) {
        m_wait_to_present_semaphores.emplace_back(&m_device, VK_SEMAPHORE_TYPE_BINARY);
    }

    m_update_cmd_buffer = std::move(CommandPoolHandler::getCommandPool(&m_device, m_device.getComputeQueue())->createCommandBuffer(1)[0]);
    
    
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(m_window, true);
    
    VkFormat format = m_swapchain.getSwapChainImage(0).getFormat();
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_device.getInstance();
    init_info.PhysicalDevice = m_device.getVkbDevice().physical_device;
    init_info.Device = m_device;
    init_info.Queue = m_device.getRenderQueue();
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
    
   
    ImGui_ImplVulkan_Init(&init_info);
    
    ImGui_ImplVulkan_CreateFontsTexture();
   

    // create renderPass

    m_deferred_renderpass = DynamicRenderPass(
        &m_device, m_window.getExtent(), {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM}, m_swapchain.getswapChainImages().size(), DEPTH, nullptr, nullptr);

 
    m_shading_renderpass = DynamicRenderPass(
        &m_device, m_window.getExtent(), {}, m_swapchain.getswapChainImages().size(), depthAndStencil::DEPTH, &m_swapchain, m_deferred_renderpass.getDepthAndStencilPtr());

        m_imgui_renderpass = DynamicRenderPass(
            &m_device, m_window.getExtent(), {}, m_swapchain.getswapChainImages().size(), depthAndStencil::NONE, &m_swapchain);
    m_imgui_renderpass.setClearEnable(false);


    m_shading_renderpass.setClearEnable(false);
    m_app->init(&m_device, &m_deferred_renderpass, &m_shading_renderpass, &m_window);
    

    m_swapchain.getSwapChainImage(0).transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL);

    // m_device wait idle
    vkDeviceWaitIdle(m_device);
}
void Engine::run() {
    // create thread for update loop

    resize();

    std::cout << "DÉBUT RENDU" << std::endl;
    std::thread update_thread(&Engine::updateLoop, std::ref(*this));

    std::thread render_thread(&Engine::renderLoop, std::ref(*this));

    update_thread.join();
    render_thread.join();
}

bool Engine::startFrame(Semaphore *&p_aquire_frame_semaphore, Fence *&p_fence) {
    const VkResult result = m_swapchain.acquireNextImage(m_current_swapchain_image, &m_render_index, p_aquire_frame_semaphore, p_fence);

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
    auto extent = m_window.getExtentGLFW();
    while (extent.width == 0 || extent.height == 0) {
        extent = m_window.getExtent();
        glfwWaitEvents();
    }
    m_resize_mutex.lock();
    vkDeviceWaitIdle(m_device);
    m_swapchain.recreateSwapchain(extent);
    m_deferred_renderpass.resize(extent);
    m_shading_renderpass.resize(extent);
    m_imgui_renderpass.resize(extent);
    m_imgui_renderpass.setClearEnable(false);
    m_app->resize(extent.width, extent.height);
    m_resize_mutex.unlock();
}

void Engine::endAndPresentFrame(Semaphore *p_wait_render_semaphore) {
    const VkResult result = m_swapchain.presentFrame(m_current_swapchain_image, p_wait_render_semaphore);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
        m_window.resetWindowResizedFlag();
        resize();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void Engine::renderLoop(Engine &p_engine) {
    auto start = std::chrono::high_resolution_clock::now();

    while (!p_engine.m_window.shouldClose()) {
        auto new_time = std::chrono::high_resolution_clock::now();

        float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - start).count();
        start = new_time;
        Semaphore *p_aquire_frame_semaphore = nullptr;
        Fence *p_fence = nullptr;

        if (!p_engine.startFrame(p_aquire_frame_semaphore, p_fence)) {
            continue;
        }
       
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        // ImGui::ShowDemoWindow();

        // DEFERRED RENDERING

        p_engine.m_deffered_render_cmd_buffers[p_engine.m_render_index].beginCommandBuffer();

        p_engine.m_app->renderDeferredFrame(
            delta_time, p_engine.m_deffered_render_cmd_buffers[p_engine.m_render_index], p_engine.m_render_index, p_engine.m_current_swapchain_image);

        p_engine.m_deffered_render_cmd_buffers[p_engine.m_render_index].endCommandBuffer();

        p_engine.m_deffered_render_cmd_buffers[p_engine.m_render_index].submitCommandBuffer(
            {}, {p_engine.m_deferred_render_semaphores[p_engine.m_render_index].getSemaphoreSubmitSignalInfo()});

        // SHADING RENDERING
        p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index].beginCommandBuffer();

        p_engine.m_swapchain.getSwapChainImage(p_engine.m_current_swapchain_image)
            .transitionImageLayout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, &p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index]);


        p_engine.m_app->renderShadedFrame(
            delta_time, p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index], p_engine.m_render_index, p_engine.m_current_swapchain_image);

        // UI RENDERING

        p_engine.m_imgui_renderpass.beginRenderPass(p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index], p_engine.m_current_swapchain_image);
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index]);
        p_engine.m_imgui_renderpass.endRenderPass(p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index]);

        p_engine.m_swapchain.getSwapChainImage(p_engine.m_current_swapchain_image)
            .transitionImageLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index]);

        p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index].endCommandBuffer();

        p_engine.m_shading_render_cmd_buffers[p_engine.m_render_index].submitCommandBuffer(
            {p_aquire_frame_semaphore->getSemaphoreSubmitWaittInfo(), p_engine.m_deferred_render_semaphores[p_engine.m_render_index].getSemaphoreSubmitSignalInfo()},
            {p_engine.m_wait_to_present_semaphores[p_engine.m_current_swapchain_image].getSemaphoreSubmitSignalInfo()}, p_fence, false);

        p_engine.endAndPresentFrame(&p_engine.m_wait_to_present_semaphores[p_engine.m_current_swapchain_image]);

      
    }
}

void Engine::updateLoop(Engine &p_engine) {
    auto start = std::chrono::high_resolution_clock::now();

    while (!p_engine.m_window.shouldClose()) {
        glfwPollEvents();
        if (p_engine.m_resize_mutex.try_lock()) {
        
            auto new_time = std::chrono::high_resolution_clock::now();
            float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - start).count();
            start = new_time;

            // p_engine.m_update_cmd_buffer.beginCommandBuffer();
            p_engine.m_app->update(delta_time, p_engine.m_update_cmd_buffer, p_engine.m_window);
            p_engine.m_resize_mutex.unlock();
        }
    }
}

}  // namespace TTe