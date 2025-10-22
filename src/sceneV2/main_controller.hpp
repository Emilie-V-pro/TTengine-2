#pragma once

#include <GLFW/glfw3.h>

#include "sceneV2/cameraV2.hpp"
#include "window.hpp"

namespace TTe {
class MainController {
   public:
   MainController(){};

   void setCursors(Window* p_window) {
       s_previous_mouse_move_callback = glfwSetCursorPosCallback(*p_window, mouseMoveCallback);
        s_previous_mouse_button_callback = glfwSetMouseButtonCallback(*p_window, mouseButtonCallback);
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
    static GLFWcursorposfun s_previous_mouse_move_callback;
    static GLFWmousebuttonfun s_previous_mouse_button_callback;


    void moveInPlaneXZ(Window* p_window, float p_dt, std::shared_ptr<CameraV2> p_cam);
    KeyMappings keys{};

    

    static void mouseMoveCallback(GLFWwindow* p_window, double p_xpos, double p_ypos);
    static void mouseButtonCallback(GLFWwindow* p_window, int p_button, int p_action, int p_mods);
    




    bool was_pressed = false;
    float move_speed{3.f};
    float look_speed{1.5f};
};
}  // namespace vk_stage