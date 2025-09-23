#pragma once 
#include <volk.h>
#include "sceneV2/node.hpp"
namespace TTe {
class CameraV2 : public Node {
   public:
    CameraV2();
    ~CameraV2();
    float fov = 80;
    float near = 0.1;
    float far = 250.0;

    VkExtent2D extent = {1, 1};
    glm::vec3 up {0,1,0};

    glm::mat4 getViewMatrix();
    glm::mat4 getInvViewMatrix();
    glm::mat4 getProjectionMatrix();
};
}  // namespace TTe