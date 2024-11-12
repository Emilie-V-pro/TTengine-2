#include "window.hpp"

#include <cstdint>

namespace TTe {
Window::Window(unsigned int width, unsigned int height, std::string name) : size(width, height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

VkSurfaceKHR &Window::getSurface(const vkb::Instance &vkInstance) {
    if (glfwCreateWindowSurface(vkInstance.instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to craete window surface");
    }
    return surface;
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto windowObj = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    windowObj->framebufferResized = true;
    windowObj->size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
}  // namespace TTe