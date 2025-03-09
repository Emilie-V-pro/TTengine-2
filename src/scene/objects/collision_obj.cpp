
#include "collision_obj.hpp"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <iostream>

namespace TTe {

void CollisionObject::collisionPosPlan(glm::vec3 &pos, glm::vec3 &vitesse) {
    // pos to Object space
    pos = glm::inverse(this->mat4()) * glm::vec4(pos, 1);

    // collision
    if (pos.y < 0) {
        vitesse = glm::vec3(0);
        pos.y = 0.0001;
    }

    pos = this->mat4() * glm::vec4(pos, 1);
}


void CollisionObject::collisionPosSphere(glm::vec3 &pos, glm::vec3 &vitesse) {
    // pos to Object space
    // std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z << "\n";
    pos = glm::inverse(this->mat4()) * glm::vec4(pos, 1);
    // std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z <<  "\n";
    // collision
    if (glm::length(pos) < 1.0) {
        if(glm::length(pos) != 0){
            pos = glm::normalize(pos);
        }else {
            pos = glm::vec3(0, 1, 0);
        }
        vitesse = glm::vec3(0);
    }
    // std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z <<  "\n";
    pos = this->mat4() * glm::vec4(pos, 1);
    // std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z <<  "\n";
}

void CollisionObject::collisionPosCube(glm::vec3 &pos, glm::vec3 &vitesse){
    
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

void CollisionObject::collisionPos(glm::vec3 &pos, glm::vec3 &vitesse) {
    switch (t) {
        case plan:
            collisionPosPlan(pos, vitesse);
            break;
        case sphere:
            collisionPosSphere(pos, vitesse);
            break;
        case cube:
            collisionPosCube(pos, vitesse);
            break;
    }
}

}  // namespace TTe