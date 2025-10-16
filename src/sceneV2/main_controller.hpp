#pragma once

#include <GLFW/glfw3.h>

#include "sceneV2/cameraV2.hpp"
#include "window.hpp"

namespace TTe {
class MainController {
   public:
   MainController(){};

   void setCursors(Window* window) {
       previousMouseMoveCallback = glfwSetCursorPosCallback(*window, mouseMoveCallback);
        previousMouseButtonCallback = glfwSetMouseButtonCallback(*window, mouseButtonCallback);
    }
    struct KeyMappings {
        int move_left = GLFW_KEY_A;
        int move_right = GLFW_KEY_D;
        int move_forward = GLFW_KEY_W;
        int move_backward = GLFW_KEY_S;
        int move_up = GLFW_KEY_E;
        int move_down = GLFW_KEY_Q;
        int look_left = GLFW_KEY_LEFT;
        int look_right = GLFW_KEY_RIGHT;
        int look_up = GLFW_KEY_UP;
        int look_down = GLFW_KEY_DOWN;
        int space = GLFW_KEY_SPACE;
        int alt = GLFW_KEY_LEFT_ALT;
        int ct = GLFW_KEY_LEFT_CONTROL;

    };

    //previous callback functions
    static GLFWcursorposfun previousMouseMoveCallback;
    static GLFWmousebuttonfun previousMouseButtonCallback;


    void moveInPlaneXZ(Window* window, float dt, std::shared_ptr<CameraV2> cam);
    KeyMappings keys{};

    

    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    




    bool waspressed = false;
    float moveSpeed{3.f};
    float lookSpeed{1.5f};
};
}  // namespace vk_stage