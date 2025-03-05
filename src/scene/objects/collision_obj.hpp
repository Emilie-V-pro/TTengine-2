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


    void collisionPosPlan(glm::vec3 &pos);
    void collisionPosSphere(glm::vec3 &pos);
    void collisionPosCube(glm::vec3 &pos);
    void collisionPos(glm::vec3 &pos);


   private:

    Type t;
};

}  // namespace TTe