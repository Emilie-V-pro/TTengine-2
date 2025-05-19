#include "portal_controller.hpp"

#include <GLFW/glfw3.h>

// std
#include <glm/gtc/constants.hpp>
#include <limits>
#include <memory>
#include "imgui.h"
#include "sceneV2/cameraV2.hpp"
#include "window.hpp"



namespace TTe {




void PortalController::moveInPlaneXZ(Window* window, float dt) {

    ImGuiIO& io = ImGui::GetIO();

    if (glfwGetKey(*window, keys.esc) == GLFW_PRESS) {
        escPressed = true;
    }
    else{
        if(escPressed){
            escPressed = false;
            focus = !focus;
            if (focus){
                window->moveCam = true;
                glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                
                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            } 
            else{
                window->moveCam = false;
                glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
        }
    }


    if(glfwGetMouseButton(*window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        leftClick = true;
    } else {
        if(leftClick){
            leftClick = false;

            const float yaw = scene->getMainCamera()->transform.rot->y;
            // transform.rot.
            const float pitch = scene->getMainCamera()->transform.rot->x;
        
            glm::vec3 forward = glm::normalize(glm::vec3{std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)});
        
            auto hit = scene->hit(scene->getMainCamera()->transform.pos, forward);
            
            if (hit.t != -1) {
                // std::cout << "hit : " << hit.t << std::endl;
                portalObjB->transform.pos = scene->getMainCamera()->transform.pos + forward * hit.t;
                portalObjB->placePortal(hit.normal, scene->getMainCamera()->transform.pos + forward * hit.t);
            }

        }
    }

    if(glfwGetMouseButton(*window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
        rightClick = true;
    } else {
        if(rightClick){
            rightClick = false;

            const float yaw = scene->getMainCamera()->transform.rot->y;
            // transform.rot.
            const float pitch = scene->getMainCamera()->transform.rot->x;
        
            glm::vec3 forward = glm::normalize(glm::vec3{std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)});
        
            auto hit = scene->hit(scene->getMainCamera()->transform.pos, forward);
            
            if (hit.t != -1) {
                // std::cout << "hit : " << hit.t << std::endl;
                portalObjA->transform.pos = scene->getMainCamera()->transform.pos + forward * hit.t;
                portalObjA->placePortal(hit.normal, scene->getMainCamera()->transform.pos + forward * hit.t);
            }

        }
    }
    



        




    std::shared_ptr<CameraV2> cam = scene->getMainCamera();
    glm::vec3 rotate{0};

    // if (glfwGetKey(*window, keys.lookRight) == GLFW_PRESS) rotate.y -= 1.f;
    // if (glfwGetKey(*window, keys.lookLeft) == GLFW_PRESS) rotate.y += 1.f;
    // if (glfwGetKey(*window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
    // if (glfwGetKey(*window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        cam->transform.rot += lookSpeed * dt * glm::normalize(rotate);
    }
    cam->transform.rot += window->mouseMove;
    window->mouseMove = glm::vec3(0);
    
    // limit pitch values between about +/- 85ish degrees
    cam->transform.rot->x = glm::clamp(cam->transform.rot->x, -1.5f, 1.5f);
    cam->transform.rot->y = glm::mod(cam->transform.rot->y, glm::two_pi<float>());

    float yaw = cam->transform.rot->y;
    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, 1.f, 0.f};

    glm::vec3 moveDir{0.f};
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
        cam->transform.pos += moveSpeed * dt * glm::normalize(moveDir);
        // std::cout << moveSpeed * dt << "\n";
    }

    
}

void PortalController::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    
    double xoffset =  windowObj->lastX - xpos ;
    double yoffset = windowObj->lastY - ypos; // reversed since y-coordinates go from bottom to top

    // std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed = now - windowObj->mouseLastMoved;
    // windowObj->mouseLastMoved = now;

    windowObj->lastX = xpos;
    windowObj->lastY = ypos;
    if(windowObj->moveCam == false) return;

    xoffset *= 0.005;
    yoffset *= 0.005;

    windowObj->mouseMove += glm::vec3(yoffset, xoffset, 0);
}

void PortalController::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    ImGuiIO& io = ImGui::GetIO();
}  // namespace TTe

}  // namespace vk_stage