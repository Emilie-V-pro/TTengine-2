
#include "collision_obj.hpp"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <limits>

namespace TTe {

void CollisionObject::collisionPosPlan(glm::vec3 &pos, glm::vec3 &vitesse) {
    // pos to Object space
    pos = glm::inverse(this->wMatrix()) * glm::vec4(pos, 1);

    // collision
    if (pos.y < 0) {
        vitesse = glm::vec3(0);
        pos.y = 0.0001;
    }

    pos = this->wMatrix()* glm::vec4(pos, 1);
}


void CollisionObject::collisionPosSphere(glm::vec3 &pos, glm::vec3 &vitesse) {
    // pos to Object space
    // std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z << "\n";
    pos = glm::inverse(this->wMatrix()) * glm::vec4(pos, 1);
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
    pos = this->wMatrix() * glm::vec4(pos, 1);
    // std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z <<  "\n";
}

void CollisionObject::collisionPosCube(glm::vec3 &pos, glm::vec3 &vitesse){
    
    pos = glm::inverse(this->wMatrix()) * glm::vec4(pos, 1);
    glm::vec3 boxMin = glm::vec3(-0.5f); // cube centré à l'origine, taille 2x2x2
    glm::vec3 boxMax = glm::vec3(0.5f);

    if (pos.x > boxMin.x && pos.x < boxMax.x &&
        pos.y > boxMin.y && pos.y < boxMax.y &&
        pos.z > boxMin.z && pos.z < boxMax.z) {
        
        // Trouver l'axe avec la plus petite distance à une face
        float dx = std::min(pos.x - boxMin.x, boxMax.x - pos.x);
        float dy = std::min(pos.y - boxMin.y, boxMax.y - pos.y);
        float dz = std::min(pos.z - boxMin.z, boxMax.z - pos.z);

        // Repousser selon l'axe le plus proche
        if (dx <= dy && dx <= dz) {
            pos.x = (pos.x > 0) ? boxMax.x : boxMin.x;
        } else if (dy <= dx && dy <= dz) {
            pos.y = (pos.y > 0) ? boxMax.y : boxMin.y;
        } else {
            pos.z = (pos.z > 0) ? boxMax.z : boxMin.z;
        }

        // Vitesse annulée (collision "parfaite")
        vitesse = glm::vec3(0);
    }

    pos = this->wMatrix() * glm::vec4(pos, 1);

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