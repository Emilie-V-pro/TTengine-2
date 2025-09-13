#pragma once

#include <glm/fwd.hpp>

#include "sceneV2/node.hpp"
namespace TTe {
class Light : public Node {
   public:
    enum LightType { DIRECTIONAL = 0, POINT = 1};
    Light() = default;
    ~Light() = default;

    glm::vec3 color {1.0f, 1.0f, 1.0f};
    float intensity {1.0f};

    LightType type;

   private:
};
}  // namespace TTe