#include "main_controller.hpp"

#include <GLFW/glfw3.h>

// std

#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>
#include <memory>
#include "imgui.h"
#include "sceneV2/cameraV2.hpp"
#include "window.hpp"



namespace TTe {

GLFWcursorposfun MainController::s_previous_mouse_move_callback = nullptr;
GLFWmousebuttonfun MainController::s_previous_mouse_button_callback = nullptr;

void MainController::moveInPlaneXZ(Window* p_window, float p_dt, std::shared_ptr<CameraV2> p_cam) {
    glm::vec3 rotate{0};

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        p_cam->transform.rot += look_speed * p_dt * glm::normalize(rotate);
    }
    p_cam->transform.rot += p_window->mouse_move;
    p_window->mouse_move = glm::vec3(0);
    
    // limit pitch values between about +/- 85ish degrees
    p_cam->transform.rot->x = glm::clamp(float(p_cam->transform.rot->x), -1.5f, 1.5f);
    p_cam->transform.rot->y = glm::mod(float(p_cam->transform.rot->y), glm::two_pi<float>());

    float yaw = p_cam->transform.rot->y;
    const glm::dvec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::dvec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::dvec3 upDir{0.f, 1.f, 0.f};

    glm::dvec3 move_dir{0.f};
    if (glfwGetKey(*p_window, keys.move_forward) == GLFW_PRESS) move_dir += forwardDir;
    if (glfwGetKey(*p_window, keys.move_backward) == GLFW_PRESS) move_dir -= forwardDir;
    if (glfwGetKey(*p_window, keys.move_right) == GLFW_PRESS) move_dir -= rightDir;
    if (glfwGetKey(*p_window, keys.move_left) == GLFW_PRESS) move_dir += rightDir;
    if (glfwGetKey(*p_window, keys.move_up) == GLFW_PRESS) move_dir += upDir;
    if (glfwGetKey(*p_window, keys.move_down) == GLFW_PRESS) move_dir -= upDir;

    if (glfwGetKey(*p_window, keys.ct) == GLFW_PRESS) {
        move_speed = 30.f;
    } else {
        move_speed = 3.f;
    }

    if (glm::dot(move_dir, move_dir) > std::numeric_limits<float>::epsilon()) {
        p_cam->transform.pos += move_speed * double(p_dt) * glm::normalize(move_dir);
        // std::cout << move_speed * dt << "\n";
    }

    
}

void MainController::mouseMoveCallback(GLFWwindow* p_window, double xpos, double ypos) {
    if(s_previous_mouse_move_callback)
        s_previous_mouse_move_callback(p_window, xpos, ypos);
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(p_window));
    
    double xoffset =  windowObj->last_x - xpos ;
    double yoffset = windowObj->last_y - ypos; // reversed since y-coordinates go from bottom to top

    // std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed = now - windowObj->mouse_last_moved;
    // windowObj->mouse_last_moved = now;

    windowObj->last_x = xpos;
    windowObj->last_y = ypos;
    if(windowObj->move_cam == false) return;

    xoffset *= 0.005;
    yoffset *= 0.005;

    windowObj->mouse_move += glm::vec3(yoffset, xoffset, 0);
}

void MainController::mouseButtonCallback(GLFWwindow* p_window, int button, int action, int mods) {
    
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(p_window));
    if(s_previous_mouse_button_callback)
        s_previous_mouse_button_callback(p_window, button, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    if(!io.WantCaptureMouse){

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if(windowObj->move_cam == false){
                windowObj->move_cam = true;
                glfwSetInputMode(p_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                
                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            if(windowObj->move_cam == true){
                windowObj->move_cam = false;
                glfwSetInputMode(p_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
        
        }

    }
}  // namespace TTe

}  // namespace vk_stage