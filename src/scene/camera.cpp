
#include "camera.hpp"

#include <glm/trigonometric.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace TTe {

glm::mat4 Camera::getViewMatrix() {
    const float yaw = transform.rot->y;
    // transform.rot.
    const float pitch = transform.rot->x;

    glm::vec3 forward{
        std::sin(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::cos(yaw) * std::cos(pitch)
    };

    // Position cible en fonction de la direction
    glm::vec3 target = transform.pos + forward;

    return glm::lookAt(transform.pos.value, target,  up);
    // const glm::vec3 w{glm::normalize(rotation)};
    // const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    // const glm::vec3 v{glm::cross(w, u)};

    // glm::mat4 viewMatrix = glm::mat4{1.f};
    // viewMatrix[0][0] = u.x;
    // viewMatrix[1][0] = u.y;
    // viewMatrix[2][0] = u.z;
    // viewMatrix[0][1] = v.x;
    // viewMatrix[1][1] = v.y;
    // viewMatrix[2][1] = v.z;
    // viewMatrix[0][2] = w.x;
    // viewMatrix[1][2] = w.y;
    // viewMatrix[2][2] = w.z;
    // viewMatrix[3][0] = -glm::dot(u, translation);
    // viewMatrix[3][1] = -glm::dot(v, translation);
    // viewMatrix[3][2] = -glm::dot(w, translation);
    // return viewMatrix;
}

glm::mat4 Camera::getInvViewMatrix() { return glm::inverse(getViewMatrix()); }

glm::mat4 Camera::getProjectionMatrix() { return glm::perspective(glm::radians(fov), ((float) extent.width / (float) extent.height), near, far); }
}  // namespace TTe