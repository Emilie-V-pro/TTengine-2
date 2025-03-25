#pragma once

#include <glm/fwd.hpp>


#include "sceneV2/Icollider.hpp"
#include "sceneV2/node.hpp"
namespace TTe {

class CollisionObject : public Node, public ICollider {
   public:
    enum Type {
        plan,
        sphere,
        cube,
    };
    
    CollisionObject() : t(Type::plan) {}
    CollisionObject(Type t) : t(t) {}

    void collisionPosPlan(glm::vec3 &pos, glm::vec3 &vitesse);
    void collisionPosSphere(glm::vec3 &pos, glm::vec3 &vitesse);
    void collisionPosCube(glm::vec3 &pos, glm::vec3 &vitesse);
    
    void collisionPos(glm::vec3 &pos, glm::vec3 &vitesse);


   private:
    Type t;
};

}  // namespace TTe