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
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
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