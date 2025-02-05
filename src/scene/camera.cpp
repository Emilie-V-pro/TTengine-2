
#include "camera.hpp"
#include <glm/trigonometric.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace TTe {

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(translation, translation+glm::normalize(rotation),up);
}

glm::mat4 Camera::getInvViewMatrix() {
    return glm::inverse(getViewMatrix());
}

glm::mat4 Camera::getProjectionMatrix(float aspect) {
    return glm::perspective(fov, aspect, near, far);
}
}