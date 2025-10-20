#pragma once


#include <memory>
#include <vector>

#include "sceneV2/Icollider.hpp"

namespace TTe {
class IAnimatic {
   public:
    virtual void simulation(glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &m_collision_objects) = 0;
   private:
   protected:
};
}  // namespace TTe