#pragma once

#include <glm/fwd.hpp>

#include "sceneV2/node.hpp"
namespace TTe {
class Light : public Node {
   public:
    enum struct LightType { DIRECTIONAL, POINT };
    Light();

    glm::vec3 color {1.0f, 1.0f, 1.0f};
    float intensity {1.0f};

    ~Light();

    LightType type;

   private:
};
}  // namespace TTe