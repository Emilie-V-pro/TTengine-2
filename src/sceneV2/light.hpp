#pragma once

#include <cstdint>
#include <glm/fwd.hpp>

#include "sceneV2/cameraV2.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class Light : public CameraV2 {
   public:
    enum LightType { DIRECTIONAL = 0, POINT = 1 };
    Light() = default;
    ~Light() = default;
    void updateMatrixFromPos(glm::vec3 p_pos);

    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity{1.0f};
    bool shadows_enabled{false};
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    virtual glm::mat4 getViewMatrix() override;
    virtual glm::mat4 getInvViewMatrix() override;
    virtual glm::mat4 getProjectionMatrix() override;
      uint32_t cam_id = 0;
    LightType m_type;

   private:
};
}  // namespace TTe