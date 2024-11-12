#pragma once

#include <glm/glm.hpp>
#include <string>

#include "volk.h"
#include "VkBootstrap.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace TTe {
class Window {
   public:
    Window(unsigned int width, unsigned int height, std::string name);
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

    // getter
    VkSurfaceKHR &getSurface (const vkb::Instance &vkInstance);
    const VkExtent2D &getExtent() const { return size; }
    const GLFWwindow *getGLFWwindow() const { return window; }
    bool wasWindowResized() const { return framebufferResized; }
    bool shouldClose() const { return glfwWindowShouldClose(window); }

    // setter
    void resetWindowResizedFlag() { framebufferResized = false; }

   private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    VkExtent2D size;

    bool framebufferResized = false;
    VkSurfaceKHR surface;
    GLFWwindow *window;
};
}  // namespace vk_stage