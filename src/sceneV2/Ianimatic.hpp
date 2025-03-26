#pragma once


#include <memory>
#include <vector>

#include "sceneV2/Icollider.hpp"

namespace TTe {
class IAnimatic {
   public:
    virtual void simulation(glm::vec3 gravite, float viscosite, int Tps, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects) = 0;
   private:
   protected:
};
}  // namespace TTe