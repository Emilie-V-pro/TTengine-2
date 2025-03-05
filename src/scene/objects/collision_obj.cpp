
#include "collision_obj.hpp"
#include <glm/common.hpp>
#include <glm/fwd.hpp>

namespace TTe {

void CollisionObject::collisionPosPlan(glm::vec3 &pos) {
    // pos to Object space
    pos = glm::inverse(this->mat4()) * glm::vec4(pos, 1);

    // collision
    if (pos.y < 0) {
        pos.y = 0;
    }

    pos = this->mat4() * glm::vec4(pos, 1);
}


void CollisionObject::collisionPosSphere(glm::vec3 &pos) {
    // pos to Object space
    pos = glm::inverse(this->mat4()) * glm::vec4(pos, 1);

    // collision
    if (glm::length(pos) > 1) {
        pos = glm::normalize(pos);
    }
    pos = this->mat4() * glm::vec4(pos, 1);
}

void CollisionObject::collisionPosCube(glm::vec3 &pos){
    
    pos = glm::inverse(this->mat4()) * glm::vec4(pos, 1);
    glm::vec3 boxPos(0.);
    glm::vec3 boxSize(1.);

    glm::vec3 adjustedRayPosition = pos - boxPos;
    
    glm::vec3 distanceVec = abs( adjustedRayPosition ) - boxSize;
    float maxDistance = glm::max( distanceVec.x , glm::max( distanceVec.y , distanceVec.z ) ); 
    float distanceToBoxSurface = glm::min( maxDistance , 0.0f ) + glm::length( glm::max( distanceVec , 0.0f ) );
    
    if( distanceToBoxSurface < 0.0f ) {
        pos = glm::normalize(pos);
    }
    pos = this->mat4() * glm::vec4(pos, 1);

}

void CollisionObject::collisionPos(glm::vec3 &pos) {
    switch (t) {
        case plan:
        case sphere:
        case cube:
            break;
    }
}

}  // namespace TTe