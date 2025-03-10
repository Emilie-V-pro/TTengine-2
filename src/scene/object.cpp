
#include "object.hpp"
#include <cmath>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/matrix.hpp>
#define GLM_FORCE_RADIANS 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace TTe {

glm::mat4 Object::mat4()  {
    
    if(dirty) {
        transform.scale += glm::vec3(0.0001);
        glm::mat4 scaleMatrix = glm::scale(transform.scale.value);
        glm::mat4 translationMatrix = glm::translate(transform.pos.value);
        glm::mat4 rotationMatrix = glm::eulerAngleXYZ(transform.rot.value.x, transform.rot.value.y, transform.rot.value.z);
        worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        dirty = false;
    }
   
    return worldMatrix;
}

glm::mat3 Object::normalMatrix() {
    if(normalDirty) {
        worldNormalMatrix = glm::inverseTranspose(glm::mat3(this->mat4()));
        normalDirty = false;
    }
    return worldNormalMatrix;
}

glm::mat4 Object::getTranslationRotationMatrix() const {
    glm::mat4 translationMatrix = glm::translate(transform.pos.value);
    glm::mat4 rotationMatrix = glm::eulerAngleXYZ(transform.rot.value.x, transform.rot.value.y, transform.rot.value.z);
    return translationMatrix * rotationMatrix;
}

glm::mat3 Object::getNormalTranslationRotationMatrix() const {
    return glm::inverseTranspose(glm::mat3(this->getTranslationRotationMatrix()));
}

glm::mat4 Object::getScaledMatrix() const {
    return glm::scale(transform.scale.value);
} 
}