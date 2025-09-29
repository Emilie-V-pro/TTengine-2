#include "main_controller_col.hpp"

#include <GLFW/glfw3.h>

// std
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <limits>
#include <memory>

#include "imgui.h"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/scene.hpp"
#include "window.hpp"

namespace TTe {

GLFWcursorposfun MainControllerCOL::previousMouseMoveCallback = nullptr;
GLFWmousebuttonfun MainControllerCOL::previousMouseButtonCallback = nullptr;

void MainControllerCOL::moveInPlaneXZ(Window* window, float dt, std::shared_ptr<CameraV2> cam, Scene* s) {
    glm::vec3 rotate{0};

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        cam->transform.rot += lookSpeed * dt * glm::normalize(rotate);
    }
    cam->transform.rot += window->mouseMove;
    window->mouseMove = glm::vec3(0);

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
        glm::vec3 ro = cam->transform.pos.value;
        glm::vec3 rd = glm::normalize(moveDir);
        glm::vec3 ro2 = cam->transform.pos.value - glm::dvec3(0,1,0);
        auto res = s->hit(ro, rd);
        auto res2 = s->hit(ro2, rd);

        float minDist;
        if (res.t != -1 && res2.t != -1) {
            minDist = std::min(res.t, res2.t);
        } else if (res.t != -1) {
            minDist = res.t;
        } else if (res2.t != -1) {
            minDist = res2.t;
        } else {
            minDist = -1;
        }

        // std::cout << res.t << "\n";
        double move_dist;
        if (minDist != -1 && minDist < 0.5) {
            move_dist = 0;
        } else {
            move_dist = moveSpeed * double(dt);
        }
        cam->transform.pos += move_dist * glm::normalize(moveDir);

        // std::cout << moveSpeed * dt << "\n";
    }
    glm::vec3 ro = cam->transform.pos.value;
    glm::vec3 rd = glm::normalize(glm::vec3(0, -1, 0));
    auto res = s->hit(ro, rd);
    double move_dist;
    if (res.t != -1 && res.t < 1.8) {
        move_dist = 0;
    } else {
        move_dist = 9.81 * double(dt);
    }

    cam->transform.pos += move_dist * glm::dvec3(rd);
}

void MainControllerCOL::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    if (previousMouseMoveCallback) previousMouseMoveCallback(window, xpos, ypos);
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    double xoffset = windowObj->lastX - xpos;
    double yoffset = windowObj->lastY - ypos;  // reversed since y-coordinates go from bottom to top

    // std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    // std::chrono::duration<double> elapsed = now - windowObj->mouseLastMoved;
    // windowObj->mouseLastMoved = now;

    windowObj->lastX = xpos;
    windowObj->lastY = ypos;
    if (windowObj->moveCam == false) return;

    xoffset *= 0.005;
    yoffset *= 0.005;

    windowObj->mouseMove += glm::vec3(yoffset, xoffset, 0);
}

void MainControllerCOL::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (previousMouseButtonCallback) previousMouseButtonCallback(window, button, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    if (!io.WantCaptureMouse) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if (windowObj->moveCam == false) {
                windowObj->moveCam = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            }
        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            if (windowObj->moveCam == true) {
                windowObj->moveCam = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
        }
    }
}  // namespace TTe

}  // namespace TTe