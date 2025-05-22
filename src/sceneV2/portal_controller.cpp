#include "portal_controller.hpp"

#include <GLFW/glfw3.h>

// std
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
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

inline float sign(float a)
{
    if (a > 0.0F) return (1.0F);
    if (a < 0.0F) return (-1.0F);
    return (0.0F);
}

glm::mat4 makeObliqueClippedProjection(
    const glm::mat4& proj, const glm::mat4& viewPortal, const glm::vec3& portalPos, const glm::vec3& portalNormal) {
    
    // 1) Plan en espace œil
    glm::vec3 P_eye = viewPortal * glm::vec4(portalPos, 1.0f);
    glm::vec3 N_eye =  glm::normalize(glm::inverseTranspose(glm::mat3(viewPortal)) * portalNormal);
    
    

    
    glm::vec4 clip_plane = glm::vec4(N_eye, -glm::length(P_eye));
    
    
    // std::cout << glm::length(P_eye) << "\n";
    std::cout << clip_plane.x << " " << clip_plane.y << " " << clip_plane.z << " " << clip_plane.w << "\n";
    clip_plane.x = -clip_plane.x; 

    // 2) Calcul du point q dans l'espace caméra
 
    glm::vec4 q = glm::vec4(
        (sign(clip_plane.x) + proj[2][0]) / proj[0][0],
        (sign(clip_plane.y) + proj[2][1]) / proj[1][1],
        -1.0f,
        (1.0f + proj[2][2]) / proj[3][2]
    );

    glm::vec4 clip_plane4 = glm::vec4(clip_plane.x, clip_plane.y, clip_plane.z, clip_plane.w);

    // 3) Mise à l'échelle du plan de coupe
    glm::vec4 c = clip_plane4 * (2.0f / glm::dot(clip_plane4, q));

    // 4) Construction de la nouvelle matrice
    glm::mat4 result = proj;
    // dans glm : result[col][row]
    result[0][2] = c.x - proj[0][3];
    result[1][2] = c.y - proj[1][3];
    result[2][2] = c.z - proj[2][3];
    result[3][2] = c.w - proj[3][3];

    return result;
}

glm::mat4 makeObliqueClippedProjection2(
    const glm::mat4& proj, const glm::mat4& viewPortal, const glm::vec3& portalPos, const glm::vec3& portalNormal) {
    
    
    glm::vec3 P_eye = viewPortal * glm::vec4(portalPos, 1.0f);
    glm::vec3 N_eye = glm::mat3(viewPortal) * portalNormal;
    N_eye = glm::normalize(N_eye);

    // 1. Plan en espace œil
    glm::vec4 plane_eye = glm::vec4(N_eye, -glm::dot(N_eye, P_eye));

    // 2. Calcul q en homogene
    glm::vec4 q;
    q.x = (glm::sign(plane_eye.x) + proj[0][2]) / proj[0][0];
    q.y = (glm::sign(plane_eye.y) + proj[1][2]) / proj[1][1];
    q.z = -1.0f;
    q.w = (1.0f + proj[2][2]) / proj[2][3];

    // 3. Calcul c
    float c = 2.0f / glm::dot(plane_eye, q);

    // 4. Nouveau plan homogene
    glm::vec4 newPlane = plane_eye * c;

    // 5. Modification de la matrice de projection
    glm::mat4 P_oblique = proj;
    // row 3 = newPlane – row 4
    P_oblique[2] = newPlane - proj[3];
    return P_oblique;
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
                portalObjB->placePortal(hit.normal, scene->getMainCamera()->transform.pos + forward * hit.t, cam->transform.pos);
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

    // // apply gravity
    // glm::vec3 move_vector = {0,0,0};

    // move_vector.y -= 9.81f * dt;

    // glm::vec3 normalized_move_vector = glm::normalize(move_vector);
    // // launche ray from camera
    // auto hit = scene->hit(scene->getMainCamera()->transform.pos, normalized_move_vector);

    // if (hit.t != -1) {
    //     move_vector = normalized_move_vector * glm::min(glm::length(move_vector), hit.t - 1.7f);
    // }

    // cam->transform.pos += move_vector;

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
        // B portal camera
        glm::mat4 view = inverse(
            portalObjB->wMatrix() * glm::rotate(glm::mat4(1.0f), glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)) *
            inverse(portalObjA->wMatrix()) * cam->getInvViewMatrix());

        glm::mat4 projection = cam->getProjectionMatrix();

        projection = makeObliqueClippedProjection(projection, view, portalObjB->transform.pos.value, portalObjB->normal);

        camData.push_back({projection, view, inverse(view)});
        // A portal camera

        view = inverse(
            portalObjA->wMatrix() * glm::rotate(glm::mat4(1.0f), glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)) *
            inverse(portalObjB->wMatrix()) * cam->getInvViewMatrix());

        projection = cam->getProjectionMatrix();
        // get the clip plane in world space

        projection = makeObliqueClippedProjection(projection, view, portalObjA->transform.pos.value, portalObjA->normal);

        camData.push_back({projection, view, inverse(view)});
    }

    if (portal_move) {
        s1->transform.pos = camData[1].invView[3];
        s2->transform.pos = camData[2].invView[3];

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