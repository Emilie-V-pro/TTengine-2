#pragma once

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include "scene/mesh.hpp"
namespace TTe {

class Object {
   public:
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};


    glm::mat4 worldMatrix;
    glm::mat3 worldNormalMatrix;

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4() const;
    glm::mat3 normalMatrix() const;

    glm::mat4 getTranslationRotationMatrix() const;
    glm::mat3 getNormalTranslationRotationMatrix() const;

    glm::mat4 getScaledMatrix() const;
    
    int parentID = -1;
    
    void draw(Mesh * mesh) const;

    uint32_t meshId;
   private:

   protected:
};
}