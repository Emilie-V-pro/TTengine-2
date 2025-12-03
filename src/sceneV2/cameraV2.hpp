#pragma once 
#include <volk.h>
#include "sceneV2/node.hpp"
namespace TTe {
class CameraV2 : public Node {
   public:

    enum Type{
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    CameraV2();
    ~CameraV2();
    float fov = 80;
    float near = 0.1;
    float far = 350.0;

    float left = -1.0;
    float right = 1.0;
    float top = 1.0;
    float bottom = -1.0;


    Type type = Type::PERSPECTIVE;
    VkExtent2D extent = {1, 1};
    glm::vec3 up {0,1,0};

    virtual glm::mat4 getViewMatrix();
    virtual glm::mat4 getInvViewMatrix();
    virtual glm::mat4 getProjectionMatrix();
};
}  // namespace TTe