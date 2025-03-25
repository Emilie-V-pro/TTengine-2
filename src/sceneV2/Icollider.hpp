#pragma once

#include <glm/fwd.hpp>
namespace TTe {
class ICollider {
   public:
    virtual void collisionPos(glm::vec3 &pos, glm::vec3 &vitesse);

   private:
   protected:
};
}  // namespace TTe