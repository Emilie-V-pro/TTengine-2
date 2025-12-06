
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
        glm::vec3 position = p_pos - direction * 200.0f;

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        this->viewMatrix = glm::lookAt(position, position + direction, up);
        this->far = 500.0f;
        this->near = -500.f;
        this->bottom = -250.0f;
        this->top = 250.0f;
        this->left = -250.0f;
        this->right = 250.0f;
        this->projectionMatrix = glm::ortho(left, right, bottom, top, near, far);

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