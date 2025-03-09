#pragma once

#include <glm/fwd.hpp>

#include "scene/object.hpp"
namespace TTe {

class CollisionObject : public Object {
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