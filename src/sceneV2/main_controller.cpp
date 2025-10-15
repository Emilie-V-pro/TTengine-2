#include "main_controller.hpp"

#include <GLFW/glfw3.h>

// std
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>
#include <memory>
#include "imgui.h"
#include "sceneV2/cameraV2.hpp"
#include "window.hpp"



namespace TTe {

GLFWcursorposfun MainController::previousMouseMoveCallback = nullptr;
GLFWmousebuttonfun MainController::previousMouseButtonCallback = nullptr;

void MainController::moveInPlaneXZ(Window* window, float dt, std::shared_ptr<CameraV2> cam) {
    glm::vec3 rotate{0};

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        cam->transform.rot += lookSpeed * dt * glm::normalize(rotate);
    }
    cam->transform.rot += window->mouse_move;
    window->mouse_move = glm::vec3(0);
    
    // limit pitch values between about +/- 85ish degrees
    cam->transform.rot->x = glm::clamp(float(cam->transform.rot->x), -1.5f, 1.5f);
    cam->transform.rot->y = glm::mod(float(cam->transform.rot->y), glm::two_pi<float>());

    float yaw = cam->transform.rot->y;
    const glm::dvec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::dvec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::dvec3 upDir{0.f, 1.f, 0.f};

    glm::dvec3 moveDir{0.f};
    if (glfwGetKey(*window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(*window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(*window, keys.moveRight) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(*window, keys.moveLeft) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(*window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(*window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glfwGetKey(*window, keys.ct) == GLFW_PRESS) {
        moveSpeed = 30.f;
    } else {
        moveSpeed = 3.f;
    }

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        cam->transform.pos += moveSpeed * double(dt) * glm::normalize(moveDir);
        // std::cout << moveSpeed * dt << "\n";
    }

    
}

void MainController::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    if(previousMouseMoveCallback)
        previousMouseMoveCallback(window, xpos, ypos);
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    
    double xoffset =  windowObj->lastX - xpos ;
    double yoffset = windowObj->lastY - ypos; // reversed since y-coordinates go from bottom to top

    // std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed = now - windowObj->mouseLastMoved;
    // windowObj->mouseLastMoved = now;

    windowObj->lastX = xpos;
    windowObj->lastY = ypos;
    if(windowObj->move_cam == false) return;

    xoffset *= 0.005;
    yoffset *= 0.005;

    windowObj->mouse_move += glm::vec3(yoffset, xoffset, 0);
}

void MainController::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if(previousMouseButtonCallback)
        previousMouseButtonCallback(window, button, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    if(!io.WantCaptureMouse){

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if(windowObj->move_cam == false){
                windowObj->move_cam = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                
                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            if(windowObj->move_cam == true){
                windowObj->move_cam = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
        
        }

    }
}  // namespace TTe

}  // namespace vk_stage