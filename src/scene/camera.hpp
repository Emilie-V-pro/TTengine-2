#pragma once

#include <glm/fwd.hpp>

#include "object.hpp"

namespace TTe {


class Camera : public Object {
   public:
    float fov = 80;
    float near = 0.01;
    float far = 100.0;
    glm::vec3 up {0,1,0};

    glm::mat4 getViewMatrix();
    glm::mat4 getInvViewMatrix();
    glm::mat4 getProjectionMatrix(float aspect);
   private:
};

}