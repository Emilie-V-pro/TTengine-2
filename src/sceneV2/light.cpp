
#include "light.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <iostream>

namespace TTe {

void Light::updateMatrixFromPos(glm::vec3 p_pos) {
    if (m_type == DIRECTIONAL) {
        glm::vec3 direction = -this->transform.rot.value;
        glm::vec3 position = glm::vec3(0) - direction * 65.0f;

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        this->viewMatrix = glm::lookAt(position, direction, up);
        this->far = 200.0f;
        this->near = 1.f;
        this->bottom = -25.0f;
        this->top = 25.0f;
        this->left = -25.0f;
        this->right = 25.0f;
        this->projectionMatrix = glm::perspective(glm::radians(30.0f), 1.0f, near, far);

    } else if (m_type == POINT) {
        // transform.pos = p_pos;
    }
}

glm::mat4 Light::getViewMatrix() {
    // std::cout << "ETETET" << std::endl;
    return this->viewMatrix; }

glm::mat4 Light::getInvViewMatrix() { return glm::inverse(this->viewMatrix); }

glm::mat4 Light::getProjectionMatrix() { return this->projectionMatrix; }

}  // namespace TTe