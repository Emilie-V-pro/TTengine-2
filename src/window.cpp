#include "window.hpp"
#include <GLFW/glfw3.h>

#include <cstdint>

namespace TTe {
Window::Window(const unsigned int p_width, const unsigned int p_height, const std::string p_name) : m_size(p_width, p_height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(p_width, p_height, p_name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

const VkSurfaceKHR &Window::createSurface(const vkb::Instance &p_vkb_instance) {
    if (glfwCreateWindowSurface(p_vkb_instance.instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to craete m_window m_surface");
    }
    return m_surface;
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

const VkExtent2D& Window::getExtentGLFW() {
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    m_size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    return m_size;
}

void Window::framebufferResizeCallback(GLFWwindow *p_window, int p_width , int p_height) {
    auto windowObj = reinterpret_cast<Window *>(glfwGetWindowUserPointer(p_window));
    windowObj->m_is_frame_buffer_resize = true;
    windowObj->m_size = {static_cast<uint32_t>(p_width), static_cast<uint32_t>(p_height)};
}
}  // namespace TTe