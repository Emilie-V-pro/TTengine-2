#pragma once

#include <glm/glm.hpp>
#include <string>
#include <chrono>

#include "volk.h"
#include "VkBootstrap.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace TTe {
class Window {
   public:
    Window(const unsigned int p_width, const unsigned int p_height, const std::string p_name);
    ~Window();

    /**
     * Only one window should be created for this engine.
     */
    // desable copy
    Window(Window const &) = delete;
    Window &operator=(Window const &) = delete;
    // desable move
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    operator GLFWwindow *() const { return m_window; }

    // getter
    const VkSurfaceKHR &createSurface(const vkb::Instance &p_vkb_instance);
    const VkExtent2D &getExtent() const { return m_size; }
    const VkExtent2D &getExtentGLFW();
    
    const GLFWwindow *getGLFWwindow() const { return m_window; }
    const bool wasWindowResized() const { return m_is_frame_buffer_resize; }
    const bool shouldClose() const { return glfwWindowShouldClose(m_window); }

    // setter
    void resetWindowResizedFlag() { m_is_frame_buffer_resize = false; }

    glm::vec3 mouse_move{0};
    bool move_cam = false;
    double last_x{0}, last_y{0};
    std::chrono::time_point<std::chrono::system_clock> mouse_last_moved;


   private:
    static void framebufferResizeCallback(GLFWwindow *p_window, int p_width, int p_height);


    VkExtent2D m_size = {0, 0};

    bool m_is_frame_buffer_resize = false;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    GLFWwindow *m_window = nullptr;
};
}  // namespace TTe