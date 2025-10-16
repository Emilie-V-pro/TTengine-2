#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

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
    Engine(IApp *p_app) : m_app(p_app) {};

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

    bool startFrame(Semaphore *&p_aquire_frame_semaphore, Fence *&p_fence);
    void endAndPresentFrame(Semaphore *p_wait_render_semaphore);

    static void renderLoop(Engine &p_engine);
    static void updateLoop(Engine &p_engine);

    void saveDeferredRenderPass();

    uint32_t m_current_swapchain_image = 0;
    int m_render_index = 0;

    bool should_close = false;

    std::mutex m_resize_mutex;

    IApp *m_app;
    Window m_window{512, 512, m_app->name};
    Device m_device{m_window};
    Swapchain m_swapchain{&m_device, m_window.getExtent(), vkb::SwapchainBuilder::BufferMode::DOUBLE_BUFFERING};

    
    std::vector<Semaphore> m_wait_to_present_semaphores;
    
    CommandBufferPool *m_cmd_buffer_pool = CommandPoolHandler::getCommandPool(&m_device, m_device.getRenderQueue());
    
    CommandBuffer m_update_cmd_buffer;
    
    // RenderData
    std::array<Semaphore, MAX_FRAMES_IN_FLIGHT>  m_deferred_render_semaphores;
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> m_deffered_render_cmd_buffers;
    std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> m_shading_render_cmd_buffers;
    
    DynamicRenderPass m_deferred_renderpass;
    DynamicRenderPass m_shading_renderpass;
    DynamicRenderPass m_imgui_renderpass;
    
};
}  // namespace TTe