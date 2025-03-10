#pragma once


#include <glm/fwd.hpp>

#include "object.hpp"

namespace TTe {


class Camera : public Object {
   public:
   Camera() {
       transform.pos = {0,0,0};
       transform.rot = glm::vec3{};
       }
    float fov = 80;
    float near = 0.01;
    float far = 1000.0;

    VkExtent2D extent = {1, 1};
    glm::vec3 up {0,1,0};

    glm::mat4 getViewMatrix();
    glm::mat4 getInvViewMatrix();
    glm::mat4 getProjectionMatrix();


   private:
};

}