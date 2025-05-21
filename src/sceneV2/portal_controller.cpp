#include "portal_controller.hpp"

#include <GLFW/glfw3.h>

// std
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/matrix.hpp>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "imgui.h"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/scene.hpp"
#include "struct.hpp"
#include "window.hpp"

namespace TTe {

glm::mat4 makeObliqueProjection(glm::mat4& proj, const glm::vec4& clipPlane_eye) {
    // Transpose pour accès ligne par ligne
    glm::mat4 P = proj;
    glm::vec4 q = glm::inverse(proj) * glm::vec4(
        glm::sign(clipPlane_eye.x),
        glm::sign(clipPlane_eye.y),
        1.0f,
        1.0f
    );
    glm::vec4 c = clipPlane_eye * (2.0f / glm::dot(clipPlane_eye, q));
    
    // Remplacement de la 3ème ligne de la matrice de projection
    proj[2] = c - proj[3];

    return P;
}

void PortalController::moveInPlaneXZ(Window* window, float dt) {
    ImGuiIO& io = ImGui::GetIO();
    std::shared_ptr<CameraV2> cam = scene->getMainCamera();


    bool portal_move = false;
    if (glfwGetKey(*window, keys.esc) == GLFW_PRESS) {
        escPressed = true;
    } else {
        if (escPressed) {
            escPressed = false;
            focus = !focus;
            if (focus) {
                window->moveCam = true;
                glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            } else {
                window->moveCam = false;
                glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
        }
    }

    if (glfwGetMouseButton(*window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        leftClick = true;
    } else {
        if (leftClick) {
            leftClick = false;

            const float yaw = scene->getMainCamera()->transform.rot->y;
            // transform.rot.
            const float pitch = scene->getMainCamera()->transform.rot->x;

            glm::vec3 forward =
                glm::normalize(glm::vec3{std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)});

            auto hit = scene->hit(scene->getMainCamera()->transform.pos, forward);

            if (hit.t != -1) {
                // std::cout << "hit : " << hit.t << std::endl;
                portalObjB->transform.pos = scene->getMainCamera()->transform.pos + forward * hit.t;
                portalObjB->placePortal(hit.normal, scene->getMainCamera()->transform.pos + forward * hit.t, cam->transform.pos );
                portal_move = true;
            }
        }
    }

    if (glfwGetMouseButton(*window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        rightClick = true;
    } else {
        if (rightClick) {
            rightClick = false;

            const float yaw = scene->getMainCamera()->transform.rot->y;
            // transform.rot.
            const float pitch = scene->getMainCamera()->transform.rot->x;

            glm::vec3 forward =
                glm::normalize(glm::vec3{std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)});

            auto hit = scene->hit(scene->getMainCamera()->transform.pos, forward);

            if (hit.t != -1) {
                // std::cout << "hit : " << hit.t << std::endl;
                portalObjA->transform.pos = scene->getMainCamera()->transform.pos + forward * hit.t;
                portalObjA->placePortal(hit.normal, scene->getMainCamera()->transform.pos + forward * hit.t, cam->transform.pos);
                portal_move = true;
            }
        }
    }

    
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

    // update camera buffer

    std::vector<Ubo> camData;
    Ubo playerCam = {cam->getProjectionMatrix(), cam->getViewMatrix(), cam->getInvViewMatrix()};
    camData.push_back(playerCam);

    // previous invView matrix

    glm::mat4 portalAView = playerCam.view;
    glm::mat4 portalBView = playerCam.view;

    // create camera data for portal view
    glm::mat4 mirrorMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));

    for (int i = 0; i < 5; i++) {


        portalObjB->transform.rot->y += M_PI; 
        // B portal camera
        glm::mat4 view = inverse(portalObjB->wMatrix()  * inverse(portalObjA->wMatrix()) * cam->getInvViewMatrix());
        portalObjB->transform.rot->y -= M_PI; 
        glm::mat4 projection = cam->getProjectionMatrix();
        
        glm::vec3 Q_eye = glm::vec3(view * glm::vec4(portalObjB->transform.pos.value, 1.0));
        glm::vec3 N_eye = glm::normalize(glm::mat3(view) * portalObjB->normal);
        glm::vec4 clipPlane_eye = glm::vec4(N_eye, -glm::dot(N_eye, Q_eye));

        // projection = makeObliqueProjection(projection, clipPlane_eye);

        camData.push_back({projection, view, inverse(view)});
        // A portal camera
        
        portalObjA->transform.rot->y += M_PI; 
        view = inverse(portalObjA->wMatrix() * mirrorMatrix *  inverse(portalObjB->wMatrix()) * cam->getInvViewMatrix());
        portalObjA->transform.rot->y -= M_PI; 
        projection = cam->getProjectionMatrix();
        Q_eye = glm::vec3(view * glm::vec4(portalObjA->transform.pos.value, 1.0));
        N_eye = glm::normalize(glm::mat3(view) * portalObjA->normal);
        clipPlane_eye = glm::vec4(N_eye, -glm::dot(N_eye, Q_eye));
        // projection = makeObliqueProjection(projection, clipPlane_eye);

        camData.push_back({projection, view, inverse(view)});
    }

    if (portal_move) {
        s1->transform.pos = camData[1].view[3];
        s2->transform.pos = camData[2].view[3];

        std::cout << "s1 pos : " << s1->transform.pos->x << " " << s1->transform.pos->y << " " << s1->transform.pos->z << "\n";
        std::cout << "s2 pos : " << s2->transform.pos->x << " " << s2->transform.pos->y << " " << s2->transform.pos->z << "\n";
    }

    // std::cout << "camera pos : " << cam->transform.pos->x << " " << cam->transform.pos->y << " " << cam->transform.pos->z << std::endl;
    // std::cout << "portala "

    scene->updateCameraBuffer(camData);
}

void PortalController::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
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

void PortalController::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* windowObj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    ImGuiIO& io = ImGui::GetIO();
}  // namespace TTe

}  // namespace TTe