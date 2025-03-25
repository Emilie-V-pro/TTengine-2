#pragma once


#include <memory>
#include <vector>

#include "sceneV2/Icollider.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
namespace TTe {
class IAnimatic {
   public:
    virtual void simulation(glm::vec3 gravite, float viscosite, int Tps, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects);
   private:
   protected:
};
}  // namespace TTe